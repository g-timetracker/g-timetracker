/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifdef WIN32
# define NOMINMAX
# include <Windows.h>
#else
# include <sys/time.h>
#endif
#include <errno.h>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDataStream>
#include <QRegularExpression>
#include <QStateMachine>
#include <QFinalState>
#include <QTimer>
#include <QFileSystemWatcher>

#include <QLoggingCategory>

#include "AbstractDataInOut.h"
#include "DataSyncerWorker.h"
#include "DBSyncer.h"

#define fail(message) \
    do {    \
        qCCritical(SYNC_WORKER_CATEGORY) << message;    \
        emit error(message);   \
    } while (0)

Q_LOGGING_CATEGORY(SYNC_WORKER_CATEGORY, "DataSyncerWorker", QtInfoMsg)

const qint32 syncFileFormatVersion = 1;
const qint32 syncFileStreamVersion = QDataStream::Qt_5_6;

const int mTimeLength = QString::number(std::numeric_limits<qint64>::max()).length();
const QString fileNamePattern = QString("(?<mTime>\\d{%1})-\\{[\\w-]+\\}").arg(mTimeLength);

const QString syncFileNamePattern = QString("^%1\\.sync$").arg(fileNamePattern);
const QRegularExpression syncFileNameRegexp(syncFileNamePattern);

const QString packFileNamePattern = QString("^%1\\.pack$").arg(fileNamePattern);
const QRegularExpression packFileNameRegexp(packFileNamePattern);

const int syncStartTimeout = 10;
const int defaultSyncCacheSize = 10;
const int fileWatchTimeout = 5;
const int defaultSyncCacheTimeout = 3600;

DataSyncerWorker::DataSyncerWorker(TimeLogHistory *db, QObject *parent) :
    QObject(parent),
    m_isInitialized(false),
    m_db(db),
    m_sm(new QStateMachine(this)),
    m_exportState(new QState()),
    m_syncFoldersState(new QState()),
    m_importState(new QState()),
    m_packSM(new QStateMachine()),
    m_packSMPackState(new QState()),
    m_packSMFinalState(new QFinalState()),
    m_timestampState(new QState()),
    m_finalState(new QFinalState()),
    m_autoSync(true),
    m_syncCacheSize(defaultSyncCacheSize),
    m_noPack(false),
    m_syncStartTimer(new QTimer(this)),
    m_syncWatcher(new QFileSystemWatcher(this)),
    m_syncWatcherTimer(new QTimer(this)),
    m_syncCacheTimeout(defaultSyncCacheTimeout),
    m_syncCacheTimer(new QTimer(this)),
    m_cachedSyncChanges(0),
    m_wroteToExternalSync(false),
    m_dbSyncer(nullptr),
    m_pack(nullptr),
    m_forcePack(false)
{
    m_exportState->addTransition(this, SIGNAL(exported()), m_syncFoldersState);
    m_syncFoldersState->addTransition(this, SIGNAL(foldersSynced()), m_importState);
    m_importState->addTransition(this, SIGNAL(imported()), m_packSM);
    m_packSMPackState->addTransition(this, SIGNAL(dirsSynced()), m_packSMFinalState);
    m_packSM->addTransition(m_packSM, SIGNAL(finished()), m_timestampState);
    m_timestampState->addTransition(this, SIGNAL(synced()), m_finalState);

    m_sm->addState(m_exportState);
    m_sm->addState(m_syncFoldersState);
    m_sm->addState(m_importState);
    m_sm->addState(m_packSM);
    m_packSM->addState(m_packSMPackState);
    m_packSM->addState(m_packSMFinalState);
    m_packSM->setInitialState(m_packSMPackState);
    m_sm->addState(m_timestampState);
    m_sm->addState(m_finalState);
    m_sm->setInitialState(m_exportState);

    connect(this, SIGNAL(started()), m_sm, SLOT(start()));
    connect(this, SIGNAL(error(QString)), m_sm, SLOT(stop()));
    connect(this, SIGNAL(started()), SLOT(startExport()), Qt::QueuedConnection);
    connect(this, SIGNAL(exported()), SLOT(syncFolders()), Qt::QueuedConnection);
    connect(this, SIGNAL(foldersSynced()), SLOT(startImport()), Qt::QueuedConnection);
    connect(m_packSM, SIGNAL(started()), this, SLOT(syncFinished()), Qt::QueuedConnection);
    connect(m_packSM, SIGNAL(finished()), SLOT(updateTimestamp()), Qt::QueuedConnection);

    connect(m_sm, SIGNAL(stopped()), this, SIGNAL(stopped()));
    connect(m_sm, SIGNAL(stopped()), this, SLOT(cleanState()));
    connect(m_sm, SIGNAL(finished()), this, SLOT(cleanState()));

    m_syncStartTimer->setTimerType(Qt::VeryCoarseTimer);
    m_syncStartTimer->setInterval(syncStartTimeout * 1000);
    m_syncStartTimer->setSingleShot(true);
    connect(m_syncStartTimer, SIGNAL(timeout()), this, SLOT(sync()));
    connect(m_sm, SIGNAL(started()), m_syncStartTimer, SLOT(stop()));

    connect(m_syncWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(syncWatcherEvent(QString)));
    m_syncWatcherTimer->setTimerType(Qt::VeryCoarseTimer);
    m_syncWatcherTimer->setInterval(fileWatchTimeout * 1000);
    m_syncWatcherTimer->setSingleShot(true);
    connect(m_syncWatcherTimer, SIGNAL(timeout()), this, SLOT(checkSyncFolder()));
    connect(m_sm, SIGNAL(started()), m_syncWatcherTimer, SLOT(stop()));

    m_syncCacheTimer->setTimerType(Qt::VeryCoarseTimer);
    m_syncCacheTimer->setInterval(m_syncCacheTimeout * 1000);
    m_syncCacheTimer->setSingleShot(true);
    connect(m_syncCacheTimer, SIGNAL(timeout()), this, SLOT(sync()));
    connect(m_sm, SIGNAL(started()), m_syncCacheTimer, SLOT(stop()));

    connect(m_db, SIGNAL(error(QString)),
            this, SLOT(historyError(QString)));
    connect(m_db, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)),
            this, SLOT(historyDataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    connect(m_db, SIGNAL(dataInserted(TimeLogEntry)),
            this, SLOT(historyDataInserted(TimeLogEntry)));
    connect(m_db, SIGNAL(dataRemoved(TimeLogEntry)),
            this, SLOT(historyDataRemoved(TimeLogEntry)));
    connect(m_db, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)),
            this, SLOT(historyCategoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));
    connect(m_db, SIGNAL(syncDataAvailable(QVector<TimeLogSyncDataEntry>,
                                           QVector<TimeLogSyncDataCategory>,QDateTime)),
            this, SLOT(syncDataAvailable(QVector<TimeLogSyncDataEntry>,
                                         QVector<TimeLogSyncDataCategory>,QDateTime)));
    connect(m_db, SIGNAL(syncExistsAvailable(bool,QDateTime,QDateTime)),
            this, SLOT(syncExistsAvailable(bool,QDateTime,QDateTime)));
    connect(m_db, SIGNAL(syncAmountAvailable(qlonglong,QDateTime,QDateTime,QDateTime)),
            this, SLOT(syncAmountAvailable(qlonglong,QDateTime,QDateTime,QDateTime)));
    connect(m_db, SIGNAL(syncEntryStatsAvailable(QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>)),
            this, SLOT(syncEntryStatsAvailable(QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>)));
    connect(m_db, SIGNAL(syncCategoryStatsAvailable(QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>)),
            this, SLOT(syncCategoryStatsAvailable(QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>)));
    connect(m_db, SIGNAL(dataSynced(QDateTime)),
            this, SLOT(syncDataSynced(QDateTime)));
}

void DataSyncerWorker::init(const QString &dataPath)
{
    m_internalSyncPath = QString("%1/sync").arg(!dataPath.isEmpty() ? dataPath
                                                                    : QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    m_isInitialized = true;
}

void DataSyncerWorker::pack(const QDateTime &start)
{
    Q_ASSERT(m_isInitialized);

    if (m_sm->isRunning() || m_packSM->isRunning()) {
        qCWarning(SYNC_WORKER_CATEGORY) << "Sync already in progress";
        return;
    } else if (m_externalSyncPath.isEmpty()) {
        fail(tr("Set sync directory"));
        return;
    }

    m_currentSyncPath = m_externalSyncPath;
    m_syncStart = start;

    m_packSM->start();
}

void DataSyncerWorker::setAutoSync(bool autoSync)
{
    if (m_autoSync == autoSync) {
        return;
    }

    m_autoSync = autoSync;

    if (m_autoSync && !m_externalSyncPath.isEmpty()) {
        checkSyncFolder();
    } else {
        m_syncStartTimer->stop();
        m_syncCacheTimer->stop();
        m_syncWatcherTimer->stop();
    }
}

void DataSyncerWorker::setSyncCacheSize(int syncCacheSize)
{
    if (m_syncCacheSize == syncCacheSize) {
        return;
    }

    m_syncCacheSize = syncCacheSize;

    if (m_autoSync && !m_externalSyncPath.isEmpty()) {
        checkCachedSyncChanges();
    }
}

void DataSyncerWorker::setSyncCacheTimeout(int syncCacheTimeout)
{
    if (m_syncCacheTimeout == syncCacheTimeout) {
        return;
    }

    m_syncCacheTimeout = syncCacheTimeout;

    if (m_syncCacheTimeout <= 0) {
        m_syncCacheTimer->stop();
    }

    if (m_syncCacheTimer->isActive()) {
        qint64 elapsedTime = m_syncCacheTimer->interval() - m_syncCacheTimer->remainingTime();
        if (elapsedTime > m_syncCacheTimeout * 1000) {
            sync();
        } else {
            m_syncCacheTimer->setInterval(m_syncCacheTimeout * 1000 - elapsedTime);
        }
    } else {
        m_syncCacheTimer->setInterval(m_syncCacheTimeout * 1000);
    }
}

void DataSyncerWorker::setSyncPath(const QString &path)
{
    if (m_externalSyncPath == path) {
        return;
    }

    if (!m_externalSyncPath.isEmpty()) {
        if (!m_syncWatcher->removePath(m_externalSyncPath)) {
            qCCritical(SYNC_WORKER_CATEGORY) << "Fail to remove directory from watcher";
        }
    }

    const QString oldPath = m_externalSyncPath;
    m_externalSyncPath = path;

    if (!m_externalSyncPath.isEmpty()) {
        if (!m_syncWatcher->addPath(m_externalSyncPath)) {
            qCCritical(SYNC_WORKER_CATEGORY) << "Fail to add directory to watcher";
        }
    }

    if (m_autoSync && !m_externalSyncPath.isEmpty()) {
        if (!oldPath.isEmpty()) {
            qCDebug(SYNC_WORKER_CATEGORY) << "Changed sync folder, force sync";
            sync();
        } else {
            checkSyncFolder();
        }
    }
}

void DataSyncerWorker::setNoPack(bool noPack)
{
    m_noPack = noPack;
}

void DataSyncerWorker::sync(const QDateTime &start)
{
    Q_ASSERT(m_isInitialized);

    if (m_sm->isRunning() || m_packSM->isRunning()) {
        qCDebug(SYNC_WORKER_CATEGORY) << "Sync already in progress";
        return;
    } else if (m_externalSyncPath.isEmpty()) {
        fail(tr("Set sync directory"));
        return;
    }

    m_currentSyncPath = m_externalSyncPath;
    m_syncStart = start;
    qCInfo(SYNC_WORKER_CATEGORY) << "Syncing with folder" << m_currentSyncPath;

    emit started(QPrivateSignal());
}

void DataSyncerWorker::historyError(const QString &errorText)
{
    Q_UNUSED(errorText);

    m_sm->stop();
}

void DataSyncerWorker::historyDataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields)
{
    Q_UNUSED(data)

    if (m_sm->isRunning()) {
        return;
    }

    for (TimeLogHistory::Fields field: fields) {
        if (field & TimeLogHistory::AllFieldsMask) {
            addCachedSyncChange();
            return;
        }
    }
}

void DataSyncerWorker::historyDataInserted(const TimeLogEntry &data)
{
    Q_UNUSED(data)

    if (m_sm->isRunning()) {
        return;
    }

    addCachedSyncChange();
}

void DataSyncerWorker::historyDataRemoved(const TimeLogEntry &data)
{
    Q_UNUSED(data)

    if (m_sm->isRunning()) {
        return;
    }

    addCachedSyncChange();
}

void DataSyncerWorker::historyCategoriesChanged(QSharedPointer<TimeLogCategoryTreeNode> categories)
{
    Q_UNUSED(categories)

    if (m_sm->isRunning()) {
        return;
    }

    addCachedSyncChange();
}

void DataSyncerWorker::syncDataAvailable(QVector<TimeLogSyncDataEntry> entryData,
                                         QVector<TimeLogSyncDataCategory> categoryData,
                                         QDateTime until)
{
    Q_UNUSED(until)

    if (!m_exportState->active()) {
        return;
    }

    if (!entryData.isEmpty() || !categoryData.isEmpty()) {
        if (!exportFile(entryData, categoryData)) {
            return;
        }
    } else {
        qCInfo(SYNC_WORKER_CATEGORY) << "No data to export";
    }

    emit exported(QPrivateSignal());
}

void DataSyncerWorker::syncExistsAvailable(bool isExists, QDateTime mBegin, QDateTime mEnd)
{
    Q_UNUSED(mBegin)
    Q_UNUSED(mEnd)

    if (isExists) {
        exportPack();
    } else {
        qCInfo(SYNC_WORKER_CATEGORY) << "No data to pack";
        emit dirsSynced(QPrivateSignal());
    }
}

void DataSyncerWorker::syncAmountAvailable(qlonglong size, QDateTime maxMTime, QDateTime mBegin, QDateTime mEnd)
{
    Q_UNUSED(mBegin)
    Q_UNUSED(mEnd)

    qCDebug(SYNC_WORKER_CATEGORY) << "Cached sync changes from DB:" << size << maxMTime;
    qint64 elapsedTime = 0;
        if (m_autoSync && !m_externalSyncPath.isEmpty() && m_syncCacheTimeout > 0 && !maxMTime.isNull()
            && (elapsedTime = maxMTime.msecsTo(QDateTime::currentDateTimeUtc())) >= m_syncCacheTimeout * 1000
            && !m_syncCacheTimer->isActive()) {
        sync();
    } else {
            if (m_autoSync && !m_externalSyncPath.isEmpty() && m_syncCacheTimeout > 0 && elapsedTime > 0
                && (!m_syncCacheTimer->isActive()
                    || m_syncCacheTimer->remainingTime() < m_syncCacheTimeout * 1000 - elapsedTime)) {
            m_syncCacheTimer->start(m_syncCacheTimeout * 1000 - elapsedTime);   // found newer entry
        }
        addCachedSyncChanges(size);
    }
}

void DataSyncerWorker::syncEntryStatsAvailable(QVector<TimeLogSyncDataEntry> removedOld,
                                               QVector<TimeLogSyncDataEntry> removedNew,
                                               QVector<TimeLogSyncDataEntry> insertedOld,
                                               QVector<TimeLogSyncDataEntry> insertedNew,
                                               QVector<TimeLogSyncDataEntry> updatedOld,
                                               QVector<TimeLogSyncDataEntry> updatedNew) const
{
    qCDebug(SYNC_WORKER_CATEGORY) << (!m_packSM->isRunning() ? "Import details:" : "Pack details:");
    for (int i = 0; i < removedNew.size(); i++) {
        qCDebug(SYNC_WORKER_CATEGORY) << formatSyncEntryChange(removedOld.at(i), removedNew.at(i));
    }
    for (int i = 0; i < insertedNew.size(); i++) {
        qCDebug(SYNC_WORKER_CATEGORY) << formatSyncEntryChange(insertedOld.at(i), insertedNew.at(i));
    }
    for (int i = 0; i < updatedNew.size(); i++) {
        qCDebug(SYNC_WORKER_CATEGORY) << formatSyncEntryChange(updatedOld.at(i), updatedNew.at(i));
    }
}

void DataSyncerWorker::syncCategoryStatsAvailable(QVector<TimeLogSyncDataCategory> removedOld,
                                                  QVector<TimeLogSyncDataCategory> removedNew,
                                                  QVector<TimeLogSyncDataCategory> addedOld,
                                                  QVector<TimeLogSyncDataCategory> addedNew,
                                                  QVector<TimeLogSyncDataCategory> updatedOld,
                                                  QVector<TimeLogSyncDataCategory> updatedNew) const
{
    qCDebug(SYNC_WORKER_CATEGORY) << (!m_packSM->isRunning() ? "Import details:" : "Pack details:");
    for (int i = 0; i < removedNew.size(); i++) {
        qCDebug(SYNC_WORKER_CATEGORY) << formatSyncCategoryChange(removedOld.at(i), removedNew.at(i));
    }
    for (int i = 0; i < addedNew.size(); i++) {
        qCDebug(SYNC_WORKER_CATEGORY) << formatSyncCategoryChange(addedOld.at(i), addedNew.at(i));
    }
    for (int i = 0; i < updatedNew.size(); i++) {
        qCDebug(SYNC_WORKER_CATEGORY) << formatSyncCategoryChange(updatedOld.at(i), updatedNew.at(i));
    }
}

void DataSyncerWorker::syncDataSynced(const QDateTime &maxSyncDate)
{
    Q_UNUSED(maxSyncDate)

    if (!m_importState->active() || m_pack /* import from pack */) {
        return;
    }

    qCInfo(SYNC_WORKER_CATEGORY) << "Successfully imported file" << m_fileList.at(m_currentIndex);

    processCurrentItemImported();
}

void DataSyncerWorker::syncFinished()
{
    if (!m_noPack) {
        packSync();
    } else {
        emit dirsSynced(QPrivateSignal());
    }
}

void DataSyncerWorker::packImported(QDateTime latestMTime)
{
    Q_UNUSED(latestMTime)

    delete m_dbSyncer;
    m_dbSyncer = nullptr;
    m_pack->deinit();
    delete m_pack;
    m_pack = nullptr;

    qCInfo(SYNC_WORKER_CATEGORY) << "Successfully imported pack" << m_fileList.at(m_currentIndex);

    processCurrentItemImported();
}

void DataSyncerWorker::packExported(QDateTime latestMTime)
{
    delete m_dbSyncer;
    m_dbSyncer = nullptr;
    m_pack->deinit();
    delete m_pack;
    m_pack = nullptr;

    QDir packDir(m_internalSyncDir.filePath("pack"));

    if (latestMTime.isNull()) {
        qCInfo(SYNC_WORKER_CATEGORY) << "No new data, leaving old pack" << m_packName;
        if (!packDir.remove("pack.pack")) {
            fail(tr("Fail to remove file %1").arg(packDir.filePath("pack.pack")));
            return;
        }
    } else {
        const QDateTime &mTime = qMax(latestMTime, m_packMTime);
        QString mTimeString = QString("%1").arg(mTime.toMSecsSinceEpoch(), mTimeLength, 10, QChar('0'));
        m_packName = QString("%1-%2.pack").arg(mTimeString).arg(QUuid::createUuid().toString());
        if (!copyFile(packDir.filePath("pack.pack"), m_internalSyncDir.filePath(m_packName), true, true)) {
            return;
        }
        if (!copyFile(m_internalSyncDir.filePath(m_packName), QDir(m_currentSyncPath).filePath(m_packName), true, false)) {
            return;
        }
        m_wroteToExternalSync = true;
        qCInfo(SYNC_WORKER_CATEGORY) << "Successfully written pack" << m_internalSyncDir.filePath(m_packName);
    }

    if (removeOldFiles(m_packName)) {
        qCInfo(SYNC_WORKER_CATEGORY) << "Successfully packed";
        emit dirsSynced(QPrivateSignal());
    }
}

void DataSyncerWorker::startImport()
{
    m_fileList = AbstractDataInOut::buildFileList(m_internalSyncDir.filePath("incoming"));
    if (m_fileList.isEmpty()) {
        qCInfo(SYNC_WORKER_CATEGORY) << "No files to import";
        emit imported(QPrivateSignal());
        return;
    }

    std::sort(m_fileList.begin(), m_fileList.end());
    std::reverse(m_fileList.begin(), m_fileList.end());
    m_currentIndex = 0;
    importCurrentItem();
}

void DataSyncerWorker::startExport()
{
    if (!AbstractDataInOut::prepareDir(m_internalSyncPath, m_internalSyncDir)) {
        fail(tr("Fail to prepare directory %1").arg(m_internalSyncPath));
        return;
    }

    QStringList fileList = AbstractDataInOut::buildFileList(m_internalSyncPath);
    QString mTimeString;
    std::sort(fileList.begin(), fileList.end());
    for (QList<QString>::const_reverse_iterator it = fileList.crbegin(); it != fileList.crend(); it++) {
        QString fileName = QFileInfo(*it).fileName();
        QRegularExpressionMatch match;
        if (!(match = syncFileNameRegexp.match(fileName)).hasMatch()
            && !(match = packFileNameRegexp.match(fileName)).hasMatch()) {
            qCInfo(SYNC_WORKER_CATEGORY) << "Skipping file not matching patterns" << fileName;
            continue;
        }

        mTimeString = match.captured("mTime");
        break;
    }

    QDateTime mFrom;
    if (!mTimeString.isEmpty()) {
        mFrom = QDateTime::fromMSecsSinceEpoch(mTimeString.toLongLong(), Qt::UTC).addMSecs(1);
    } else {
        QDateTime::fromMSecsSinceEpoch(0, Qt::UTC);
    }

    m_db->getSyncData(mFrom);
}

void DataSyncerWorker::syncFolders()
{
    compareWithDir(m_currentSyncPath);

    qCDebug(SYNC_WORKER_CATEGORY) << "Out files:" << m_outFiles;
    qCDebug(SYNC_WORKER_CATEGORY) << "In files:" << m_inFiles;

    if (!copyFiles(m_internalSyncDir.path(), m_currentSyncPath, m_outFiles, false)
        || !copyFiles(m_currentSyncPath, m_internalSyncDir.filePath("incoming"), m_inFiles, false)) {
        return;
    }
    if (!m_outFiles.isEmpty()) {
        m_wroteToExternalSync = true;
    }

    qCInfo(SYNC_WORKER_CATEGORY) << "Folders synced";

    emit foldersSynced(QPrivateSignal());
}

void DataSyncerWorker::packSync()
{
    QStringList fileList = AbstractDataInOut::buildFileList(m_internalSyncPath);
    std::reverse(fileList.begin(), fileList.end());

    // TODO: refactor to std::find_if when buildFileList() would return QFileInfoList
    QString packName;
    auto it = fileList.cbegin();
    while (it != fileList.cend()) {
        QString fileName = QFileInfo(*it).fileName();
        if (packFileNameRegexp.match(fileName).hasMatch()) {
            packName = fileName;
            break;
        } else {
            ++it;
        }
    }

    QString packMTimeString;
    if (m_forcePack && !m_packName.isEmpty()) {
        packMTimeString = packFileNameRegexp.match(m_packName).captured("mTime");
    }

    if (!packName.isEmpty()) {
        if (m_packName != packName) {
            QString lastMTimeString = packFileNameRegexp.match(packName).captured("mTime");
            if (lastMTimeString > packMTimeString) {    // Use pack with latest mTime
                m_packName = packName;
                packMTimeString = lastMTimeString;
            }
        }
        m_packMTime = QDateTime::fromMSecsSinceEpoch(packMTimeString.toLongLong(), Qt::UTC);
    } else {
        m_packMTime = QDateTime::fromMSecsSinceEpoch(0, Qt::UTC);
    }

    QDateTime packPeriodStart(maxPackPeriodStart());
    if (m_forcePack) {
        exportPack();
    } else if (m_packMTime < packPeriodStart) {
        m_db->getSyncExists(m_packMTime, packPeriodStart.addMonths(1).addMSecs(-1));
    } else {
        emit dirsSynced(QPrivateSignal());
    }
}

void DataSyncerWorker::updateTimestamp()
{
    if (m_wroteToExternalSync) {
        qCDebug(SYNC_WORKER_CATEGORY) << "Wrote to sync folder, updating mtime";
#ifndef WIN32
        if (utimes(m_internalSyncPath.toLocal8Bit().constData(), NULL) != 0) {
            qCCritical(SYNC_WORKER_CATEGORY) << QString("utimes failed, errno: %1").arg(QString().setNum(errno));
#else
        SYSTEMTIME systemTime;
        FILETIME fileTime;
        ::GetSystemTime(&systemTime);
        ::SystemTimeToFileTime(&systemTime, &fileTime);

        HANDLE fileHandle = ::CreateFile((const wchar_t*) QDir::toNativeSeparators(m_internalSyncPath).utf16(),
                                         GENERIC_READ | GENERIC_WRITE,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         NULL,
                                         OPEN_EXISTING,
                                         FILE_FLAG_BACKUP_SEMANTICS,
                                         NULL);
        if (fileHandle == INVALID_HANDLE_VALUE) {
            qCCritical(SYNC_WORKER_CATEGORY) << QString("CreateFile failed, error code: %1").arg(QString().setNum(::GetLastError()));
            fail(tr("Fail to update directory timestamp"));
        }
        bool result = ::SetFileTime(fileHandle, NULL, NULL, &fileTime);
        ::CloseHandle(fileHandle);
        if (!result) {
            qCCritical(SYNC_WORKER_CATEGORY) << QString("SetFileTime failed, error code: %1").arg(QString().setNum(::GetLastError()));
#endif
            fail(tr("Fail to update directory timestamp"));
            return;
        }
    }

    emit synced(QPrivateSignal());
}

void DataSyncerWorker::cleanState()
{
    m_fileList.clear();
    m_currentIndex = 0;
    m_outFiles.clear();
    m_inFiles.clear();
    m_cachedSyncChanges = 0;
    m_wroteToExternalSync = false;

    if (m_dbSyncer) {
        qCCritical(SYNC_WORKER_CATEGORY) << "m_dbSyncer is not null" << m_dbSyncer;
        delete m_dbSyncer;
        m_dbSyncer = nullptr;
    }
    if (m_pack) {
        qCCritical(SYNC_WORKER_CATEGORY) << "m_pack is not null" << m_pack;
        delete m_pack;
        m_pack = nullptr;
    }

    m_packName.clear();
    m_packMTime = QDateTime();
    m_forcePack = false;
}

void DataSyncerWorker::checkSyncFolder()
{
    QFileInfo dataDirInfo(m_internalSyncPath);
    QFileInfo syncDirInfo(m_externalSyncPath);

    if ((!dataDirInfo.exists() && syncDirInfo.exists()) // initial sync
        || dataDirInfo.lastModified() < syncDirInfo.lastModified()) {
        qCDebug(SYNC_WORKER_CATEGORY) << "Sync folder is newer, sync needed"
                                      << dataDirInfo.lastModified() << syncDirInfo.lastModified();
        if (m_autoSync) {
            sync();
        }
    } else {
        m_db->getSyncAmount(dataDirInfo.lastModified().addMSecs(1));
    }
}

void DataSyncerWorker::syncWatcherEvent(const QString &path)
{
    qCDebug(SYNC_WORKER_CATEGORY) << "Event for sync directory" << path;

    if (m_autoSync) {
        m_syncWatcherTimer->start();
    }
}

void DataSyncerWorker::compareWithDir(const QString &path)
{
    QDir dir(path);
    QSet<QString> extEntries =  dir.entryList(QDir::Files).toSet();
    QSet<QString> intEntries = m_internalSyncDir.entryList(QDir::Files).toSet();

    m_outFiles = QSet<QString>(intEntries).subtract(extEntries);
    m_inFiles = QSet<QString>(extEntries).subtract(intEntries);
}

bool DataSyncerWorker::copyFiles(const QString &from, const QString &to, const QSet<QString> fileList, bool isRemoveSource)
{
    QDir sourceDir(from);
    QDir destinationDir;
    if (!AbstractDataInOut::prepareDir(to, destinationDir)) {
        fail(tr("Fail to prepare directory %1").arg(to));
        return false;
    }

    foreach (const QString fileName, fileList) {
        if (!copyFile(sourceDir.filePath(fileName), destinationDir.filePath(fileName), true, isRemoveSource)) {
            return false;
        }
    }

    return true;
}

bool DataSyncerWorker::copyFile(const QString &source, const QString &destination, bool isOverwrite, bool isRemoveSource) const
{
    QFile sourceFile(source);
    QFile destinationFile(destination);
    if (destinationFile.exists() && isOverwrite) {
        qCInfo(SYNC_WORKER_CATEGORY) << QString("File %1 already exists, removing")
                                        .arg(destinationFile.fileName());
        if (!destinationFile.remove()) {
            qCCritical(SYNC_WORKER_CATEGORY)
                    << AbstractDataInOut::formatFileError("Fail to remove file", destinationFile);
            fail(tr("Fail to remove file %1").arg(destinationFile.fileName()));
            return false;
        }
    }
    if (!(isRemoveSource ? sourceFile.rename(destinationFile.fileName())
                         : sourceFile.copy(destinationFile.fileName()))) {
        qCCritical(SYNC_WORKER_CATEGORY)
                << AbstractDataInOut::formatFileError(QString("Fail to %1 file to %2 from")
                                                      .arg(isRemoveSource ? "move" : "copy")
                                                      .arg(destinationFile.fileName()),
                                                      sourceFile);
        fail((isRemoveSource ? tr("Fail to move file %1 to %2")
                             : tr("Fail to copy file %1 to %2"))
             .arg(sourceFile.fileName()).arg(destinationFile.fileName()));
        return false;
    }

    return true;
}

bool DataSyncerWorker::exportFile(const QVector<TimeLogSyncDataEntry> &entryData,
                                  const QVector<TimeLogSyncDataCategory> &categoryData)
{
    QDir exportDir;
    if (!AbstractDataInOut::prepareDir(m_internalSyncDir.filePath("export"), exportDir)) {
        fail(tr("Fail to prepare directory %1").arg(m_internalSyncDir.filePath("export")));
        return false;
    }

    QDateTime maxMTime(qMax(entryData.isEmpty() ? QDateTime() : entryData.last().sync.mTime,
                            categoryData.isEmpty() ? QDateTime() : categoryData.last().sync.mTime));
    QString fileName = QString("%1-%2.sync").arg(maxMTime.toMSecsSinceEpoch(), mTimeLength, 10, QChar('0'))
                                            .arg(QUuid::createUuid().toString());
    QString filePath = exportDir.filePath(fileName);

    qCInfo(SYNC_WORKER_CATEGORY) << "Exporting data to file" << filePath;

    QByteArray fileData;
    QDataStream dataStream(&fileData, QIODevice::WriteOnly);
    dataStream.setVersion(syncFileStreamVersion);
    dataStream << syncFileFormatVersion;

    for (int i = 0; i < entryData.size(); i++) {
        qCInfo(SYNC_WORKER_CATEGORY) << QString("Exported entry %1 of %2").arg(i+1).arg(entryData.size());
        qCDebug(SYNC_WORKER_CATEGORY) << entryData.at(i).toString();
        dataStream << entryData.at(i);
    }

    for (int i = 0; i < categoryData.size(); i++) {
        qCInfo(SYNC_WORKER_CATEGORY) << QString("Exported category %1 of %2").arg(i+1).arg(categoryData.size());
        qCDebug(SYNC_WORKER_CATEGORY) << categoryData.at(i).toString();
        dataStream << categoryData.at(i);
    }

    QFile file(filePath);
    if (file.exists()) {
        qCWarning(SYNC_WORKER_CATEGORY) << "File already exists, overwriting" << filePath;
        return false;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCCritical(SYNC_WORKER_CATEGORY) << AbstractDataInOut::formatFileError("Fail to open file", file);
        fail(tr("Fail to open file %1").arg(file.fileName()));
        return false;
    }

    QDataStream fileStream(&file);
    fileStream << syncFileStreamVersion;
    fileStream.setVersion(syncFileStreamVersion);

    fileStream << fileData;
    fileStream << qChecksum(fileData.constData(), fileData.size());

    if (fileStream.status() != QDataStream::Ok || file.error() != QFileDevice::NoError) {
        qCCritical(SYNC_WORKER_CATEGORY) << AbstractDataInOut::formatFileError("Error writing to file", file);
        fail(tr("Fail to write to file %1").arg(file.fileName()));
        return false;
    }

    file.close();

    if (!copyFile(filePath, m_internalSyncDir.filePath(fileName), true, true)) {
        return false;
    }

    qCInfo(SYNC_WORKER_CATEGORY) << "All data successfully exported";

    return true;
}

void DataSyncerWorker::importCurrentItem()
{
    qCInfo(SYNC_WORKER_CATEGORY) << QString("Importing file %1 of %2")
                                    .arg(m_currentIndex+1).arg(m_fileList.size());
    const QString &filePath(m_fileList.at(m_currentIndex));
    if (packFileNameRegexp.match(QFileInfo(filePath).fileName()).hasMatch()) {
        m_forcePack = true;
        m_packName = QFileInfo(filePath).fileName();
        importPack(filePath);
    } else {
        importFile(filePath);
    }
}

void DataSyncerWorker::importFile(const QString &path)
{
    QVector<TimeLogSyncDataEntry> updatedData, removedData;
    QVector<TimeLogSyncDataCategory> categoryData;

    if (!parseFile(path, updatedData, removedData, categoryData)) {
        return;
    }

    if (!updatedData.isEmpty() || !removedData.isEmpty() || !categoryData.isEmpty()) {
        m_db->sync(updatedData, removedData, categoryData);
    } else {
        syncDataSynced(QDateTime());
    }
}

bool DataSyncerWorker::parseFile(const QString &path,
                                 QVector<TimeLogSyncDataEntry> &updatedData,
                                 QVector<TimeLogSyncDataEntry> &removedData,
                                 QVector<TimeLogSyncDataCategory> &categoryData) const
{
    QFile file(path);
    if (!file.exists()) {
        fail(tr("File %1 does not exists").arg(path));
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCCritical(SYNC_WORKER_CATEGORY) << AbstractDataInOut::formatFileError("Fail to open file", file);
        fail(tr("Fail to open file %1").arg(file.fileName()));
        return false;
    }

    QDataStream fileStream(&file);

    if (fileStream.atEnd()) {
        fail(tr("Invalid file %1, no stream version").arg(path));
        return false;
    }
    qint32 streamVersion;
    fileStream >> streamVersion;
    if (streamVersion > syncFileStreamVersion) {
        fail(tr("Stream version too new: %1 > %2")
             .arg(streamVersion).arg(syncFileStreamVersion));
        return false;
    }
    fileStream.setVersion(streamVersion);

    if (fileStream.atEnd()) {
        fail(tr("Invalid file %1, no data").arg(path));
        return false;
    }
    QByteArray fileData;
    fileStream >> fileData;
    if (fileStream.status() != QDataStream::Ok || file.error() != QFileDevice::NoError) {
        qCCritical(SYNC_WORKER_CATEGORY)
                << AbstractDataInOut::formatFileError(QString("Error reading from file, stream status %1")
                                                      .arg(fileStream.status()), file);
        fail(tr("Fail to read from file %1").arg(file.fileName()));
        return false;
    }

    if (fileStream.atEnd()) {
        fail(tr("Invalid file %1, no checksum").arg(path));
        return false;
    }
    quint16 checksum;
    fileStream >> checksum;
    if (qChecksum(fileData.constData(), fileData.size()) != checksum) {
        fail(tr("Invalid file %1, checksums does not match").arg(path));
        return false;
    }

    QDataStream dataStream(&fileData, QIODevice::ReadOnly);
    dataStream.setVersion(streamVersion);

    if (dataStream.atEnd()) {
        fail(tr("Invalid data in file %1, no format version").arg(path));
        return false;
    }
    qint32 formatVersion;
    dataStream >> formatVersion;
    if (formatVersion != syncFileFormatVersion) {
        fail(tr("Data format version %1 instead of %2")
             .arg(formatVersion).arg(syncFileFormatVersion));
        return false;
    }

    while (!dataStream.atEnd()) {
        TimeLogSyncDataBase base;
        dataStream >> base;

        switch (base.type) {
        case TimeLogSyncDataBase::Entry: {
            TimeLogEntry entry;
            dataStream >> entry;
            TimeLogSyncDataEntry syncEntry(base, entry);

            if (entry.isValid()) {
                syncEntry.sync.isRemoved = false;
                qCDebug(SYNC_WORKER_CATEGORY) << "Valid entry:" << syncEntry;
                updatedData.append(syncEntry);
            } else if (!entry.uuid.isNull() && base.mTime.isValid()) {
                syncEntry.sync.isRemoved = true;
                qCDebug(SYNC_WORKER_CATEGORY) << "Valid removed entry:" << syncEntry;
                removedData.append(syncEntry);
            } else {
                qCWarning(SYNC_WORKER_CATEGORY) << "Invalid entry:" << syncEntry;
                fail(tr("Invalid sync entry"));
                return false;
            }
            break;
        }
        case TimeLogSyncDataBase::Category: {
            TimeLogCategory category;
            dataStream >> category;
            TimeLogSyncDataCategory syncCategory(base, category);

            if (category.isValid()) {
                syncCategory.sync.isRemoved = false;
                qCDebug(SYNC_WORKER_CATEGORY) << "Valid category:" << syncCategory;
                categoryData.append(syncCategory);
            } else if (!category.uuid.isNull() && base.mTime.isValid()) {
                syncCategory.sync.isRemoved = true;
                qCDebug(SYNC_WORKER_CATEGORY) << "Valid removed category:" << syncCategory;
                categoryData.append(syncCategory);
            } else {
                qCWarning(SYNC_WORKER_CATEGORY) << "Invalid category:" << syncCategory;
                fail(tr("Invalid sync category"));
                return false;
            }
            break;
        }
        default:
            qCWarning(SYNC_WORKER_CATEGORY) << "Invalid sync item type:" << base.type;
            fail(tr("Invalid sync item type"));
            return false;
        }
    }

    if (updatedData.isEmpty() && removedData.isEmpty() && categoryData.isEmpty()) {
        qCWarning(SYNC_WORKER_CATEGORY) << "No data in file" << path;
    }

    return true;
}

void DataSyncerWorker::importPack(const QString &path)
{
    m_pack = new TimeLogHistory(this);
    if (!m_pack->init(m_internalSyncPath, m_internalSyncDir.relativeFilePath(path), true)) {
        fail(tr("Fail to open pack file %1").arg(path));
        return;
    }

    m_dbSyncer = new DBSyncer(m_pack, m_db, this);
    connect(m_dbSyncer, SIGNAL(finished(QDateTime)),
            this, SLOT(packImported(QDateTime)));
    connect(m_dbSyncer, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));

    m_dbSyncer->start(false);
}

void DataSyncerWorker::processCurrentItemImported()
{
    ++m_currentIndex;
    if (m_currentIndex == m_fileList.size()) {
        if (copyFiles(m_internalSyncDir.filePath("incoming"), m_internalSyncDir.path(), m_inFiles, true)) {
            emit imported(QPrivateSignal());
        }
    } else {
        importCurrentItem();
    }
}

void DataSyncerWorker::exportPack()
{
    QDir packDir;
    if (!AbstractDataInOut::prepareDir(m_internalSyncDir.filePath("pack"), packDir)) {
        fail(tr("Fail to prepare directory %1").arg(m_internalSyncDir.filePath("pack")));
        return;
    }

    if (!m_packName.isEmpty()) {
        if (!copyFile(m_internalSyncDir.filePath(m_packName), packDir.filePath("pack.pack"), true, false)) {
            fail(tr("Fail to copy pack file to %1").arg(packDir.filePath("pack.pack")));
            return;
        }
        qCDebug(SYNC_WORKER_CATEGORY) << "Using existing pack file" << m_packName;
    } else {
        qCDebug(SYNC_WORKER_CATEGORY) << "No existing pack file, creating new";
    }

    m_pack = new TimeLogHistory(this);
    if (!m_pack->init(m_internalSyncPath,
                      m_internalSyncDir.relativeFilePath(packDir.filePath("pack.pack")),
                      false)) {
        fail(tr("Fail to create pack file"));
        return;
    }
    connect(m_pack, SIGNAL(syncEntryStatsAvailable(QVector<TimeLogSyncDataEntry>,
                                                   QVector<TimeLogSyncDataEntry>,
                                                   QVector<TimeLogSyncDataEntry>,
                                                   QVector<TimeLogSyncDataEntry>,
                                                   QVector<TimeLogSyncDataEntry>,
                                                   QVector<TimeLogSyncDataEntry>)),
            this, SLOT(syncEntryStatsAvailable(QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataEntry>)));
    connect(m_pack, SIGNAL(syncCategoryStatsAvailable(QVector<TimeLogSyncDataCategory>,
                                                      QVector<TimeLogSyncDataCategory>,
                                                      QVector<TimeLogSyncDataCategory>,
                                                      QVector<TimeLogSyncDataCategory>,
                                                      QVector<TimeLogSyncDataCategory>,
                                                      QVector<TimeLogSyncDataCategory>)),
            this, SLOT(syncCategoryStatsAvailable(QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>,
                                                  QVector<TimeLogSyncDataCategory>)));

    m_dbSyncer = new DBSyncer(m_db, m_pack, this);
    connect(m_dbSyncer, SIGNAL(finished(QDateTime)),
            this, SLOT(packExported(QDateTime)));
    connect(m_dbSyncer, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));

    m_dbSyncer->start(true, maxPackPeriodStart());
}

QString DataSyncerWorker::formatSyncEntryChange(const TimeLogSyncDataEntry &oldData,
                                                const TimeLogSyncDataEntry &newData) const
{
    QStringList result;

    if (newData.entry.isValid()) {
        if (!oldData.entry.uuid.isNull()) {
            result << "[Updated]" << oldData.toString() << "->" << newData.toString();
        } else {
            result << "[Added]" << newData.toString();
        }
    } else {
        result << "[Removed]" << oldData.toString() << "->" << newData.toString();
    }

    return result.join(' ');
}

QString DataSyncerWorker::formatSyncCategoryChange(const TimeLogSyncDataCategory &oldData,
                                                   const TimeLogSyncDataCategory &newData) const
{
    QStringList result;

    if (newData.category.isValid()) {
        if (!oldData.category.uuid.isNull()) {
            result << "[Updated]" << oldData.toString() << "->" << newData.toString();
        } else {
            result << "[Added]" << newData.toString();
        }
    } else {
        result << "[Removed]" << oldData.toString() << "->" << newData.toString();
    }

    return result.join(' ');
}

QDateTime DataSyncerWorker::maxPackPeriodStart() const
{
    QDate date = qMax(m_syncStart.toUTC().date().addMonths(-1), m_packMTime.toUTC().date());
    return QDateTime(QDate(date.year(), date.month(), 1), QTime(), Qt::UTC);
}

bool DataSyncerWorker::removeOldFiles(const QString &packName)
{
    QString packMTimeString = packFileNameRegexp.match(packName).captured("mTime");
    QStringList fileList = AbstractDataInOut::buildFileList(m_internalSyncPath);
    for (const QString &filePath: fileList) {
        QString fileName = QFileInfo(filePath).fileName();
        if (fileName == packName) { // Don't delete last pack
            continue;
        }
        QRegularExpressionMatch match;
        if ((match = packFileNameRegexp.match(fileName)).hasMatch()) {
            if (match.captured("mTime") > packMTimeString) {
                continue;
            }
        } else if ((match = syncFileNameRegexp.match(fileName)).hasMatch()) {
            if (match.captured("mTime") > packMTimeString) {
                continue;
            }
        } else {
            continue;
        }

        if (!m_internalSyncDir.remove(fileName)) {
            fail(QString("Fail to remove file %1").arg(filePath));
            return false;
        }
        qCDebug(SYNC_WORKER_CATEGORY) << "Removed" << m_internalSyncDir.filePath(fileName);
    }

    QDir syncDir(m_currentSyncPath);
    fileList = AbstractDataInOut::buildFileList(m_currentSyncPath);
    for (const QString &filePath: fileList) {
        QString fileName = QFileInfo(filePath).fileName();
        if (fileName == packName) { // Don't delete last pack
            continue;
        }
        QRegularExpressionMatch match;
        if ((match = packFileNameRegexp.match(fileName)).hasMatch()) {
            if (match.captured("mTime") > packMTimeString) {
                continue;
            }
        } else if ((match = syncFileNameRegexp.match(fileName)).hasMatch()) {
            if (match.captured("mTime") > packMTimeString) {
                continue;
            }
        } else {
            continue;
        }

        if (!syncDir.remove(fileName)) {
            fail(tr("Fail to remove file %1").arg(filePath));
            return false;
        }
        m_wroteToExternalSync = true;

        qCDebug(SYNC_WORKER_CATEGORY) << "Removed" << syncDir.filePath(fileName);
    }

    return true;
}

void DataSyncerWorker::addCachedSyncChange()
{
    if (m_autoSync && m_syncCacheTimeout > 0) {
        m_syncCacheTimer->start(m_syncCacheTimeout * 1000);
    }

    addCachedSyncChanges(1);
}

void DataSyncerWorker::addCachedSyncChanges(int count)
{
    m_cachedSyncChanges += count;

    qCDebug(SYNC_WORKER_CATEGORY) << "Cached sync changes:" << m_cachedSyncChanges;

    checkCachedSyncChanges();
}

void DataSyncerWorker::checkCachedSyncChanges()
{
    if (!m_autoSync || m_sm->isRunning() || m_externalSyncPath.isEmpty()) {
        return;
    } else if (m_cachedSyncChanges > m_syncCacheSize) {
        m_syncStartTimer->start();
    }
}
