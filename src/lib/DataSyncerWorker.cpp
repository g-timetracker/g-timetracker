#include <QCoreApplication>
#include <QStandardPaths>
#include <QDataStream>
#include <QRegularExpression>
#include <QStateMachine>
#include <QFinalState>

#include <QLoggingCategory>

#include "AbstractDataInOut.h"
#include "DataSyncerWorker.h"
#include "TimeLogHistory.h"
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

DataSyncerWorker::DataSyncerWorker(TimeLogHistory *db, QObject *parent) :
    QObject(parent),
    m_isInitialized(false),
    m_db(db),
    m_sm(new QStateMachine(this)),
    m_exportState(new QState()),
    m_syncFoldersState(new QState()),
    m_importState(new QState()),
    m_packState(new QState()),
    m_finalState(new QFinalState()),
    m_noPack(false),
    m_dbSyncer(nullptr),
    m_pack(nullptr),
    m_forcePack(false)
{
    m_exportState->addTransition(this, SIGNAL(exported()), m_syncFoldersState);
    m_syncFoldersState->addTransition(this, SIGNAL(foldersSynced()), m_importState);
    m_importState->addTransition(this, SIGNAL(imported()), m_packState);
    m_packState->addTransition(this, SIGNAL(synced()), m_finalState);

    m_sm->addState(m_exportState);
    m_sm->addState(m_syncFoldersState);
    m_sm->addState(m_importState);
    m_sm->addState(m_packState);
    m_sm->addState(m_finalState);
    m_sm->setInitialState(m_exportState);

    connect(this, SIGNAL(started()), m_sm, SLOT(start()));
    connect(this, SIGNAL(error(QString)), m_sm, SLOT(stop()));
    connect(this, SIGNAL(started()), SLOT(startExport()), Qt::QueuedConnection);
    connect(this, SIGNAL(exported()), SLOT(syncFolders()), Qt::QueuedConnection);
    connect(this, SIGNAL(foldersSynced()), SLOT(startImport()), Qt::QueuedConnection);
    connect(this, SIGNAL(imported()), SLOT(syncFinished()), Qt::QueuedConnection);

    connect(m_sm, SIGNAL(stopped()), this, SLOT(cleanState()));
    connect(m_sm, SIGNAL(finished()), this, SLOT(cleanState()));

    connect(m_db, SIGNAL(error(QString)),
            this, SLOT(historyError(QString)));
    connect(m_db, SIGNAL(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)),
            this, SLOT(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)));
    connect(m_db, SIGNAL(hasSyncData(bool,QDateTime,QDateTime)),
            this, SLOT(syncHasSyncData(bool,QDateTime,QDateTime)));
    connect(m_db, SIGNAL(syncStatsAvailable(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                            QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                            QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)),
            this, SLOT(syncStatsAvailable(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                          QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                          QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)));
    connect(m_db, SIGNAL(dataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)),
            this, SLOT(syncDataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)));
}

void DataSyncerWorker::init(const QString &dataPath)
{
    m_intPath = QString("%1/sync").arg(!dataPath.isEmpty() ? dataPath
                                                           : QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    m_isInitialized = true;
}

void DataSyncerWorker::sync(const QString &path, const QDateTime &start)
{
    Q_ASSERT(m_isInitialized);

    if (m_sm->isRunning() || m_pack) {  // TODO: pack state machine
        qCWarning(SYNC_WORKER_CATEGORY) << "Sync already running";
        return;
    } else if (path.isEmpty()) {
        fail("Empty sync path");
        return;
    }

    m_syncPath = path;
    m_syncStart = start;
    qCInfo(SYNC_WORKER_CATEGORY) << "Syncing with folder" << m_syncPath;

    emit started(QPrivateSignal());
}

void DataSyncerWorker::setNoPack(bool noPack)
{
    m_noPack = noPack;
}

void DataSyncerWorker::pack(const QString &path, const QDateTime &start)
{
    Q_ASSERT(m_isInitialized);

    if (m_sm->isRunning() || m_pack) {  // TODO: pack state machine
        qCWarning(SYNC_WORKER_CATEGORY) << "Sync already in progress";
        return;
    }

    m_syncPath = path;
    m_syncStart = start;

    packSync();
}

void DataSyncerWorker::historyError(const QString &errorText)
{
    Q_UNUSED(errorText);

    m_sm->stop();
}

void DataSyncerWorker::syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until)
{
    Q_UNUSED(until)

    if (!m_exportState->active() || m_pack) { // sync with pack
        return;
    }

    if (!data.isEmpty()) {
        if (!exportFile(data)) {
            return;
        }
    } else {
        qCInfo(SYNC_WORKER_CATEGORY) << "No data to export";
    }

    emit exported(QPrivateSignal());
}

void DataSyncerWorker::syncHasSyncData(bool hasData, QDateTime mBegin, QDateTime mEnd)
{
    Q_UNUSED(mBegin)
    Q_UNUSED(mEnd)

    if (hasData) {
        exportPack();
    } else {
        qCInfo(SYNC_WORKER_CATEGORY) << "No data to pack";
        emit synced(QPrivateSignal());
    }
}

void DataSyncerWorker::syncStatsAvailable(QVector<TimeLogSyncData> removedOld, QVector<TimeLogSyncData> removedNew, QVector<TimeLogSyncData> insertedOld, QVector<TimeLogSyncData> insertedNew, QVector<TimeLogSyncData> updatedOld, QVector<TimeLogSyncData> updatedNew) const
{
    qCDebug(SYNC_WORKER_CATEGORY) << (sender() == m_db ? "Import details:" : "Pack details:");
    for (int i = 0; i < removedNew.size(); i++) {
        qCDebug(SYNC_WORKER_CATEGORY) << formatSyncChange(removedOld.at(i), removedNew.at(i));
    }
    for (int i = 0; i < insertedNew.size(); i++) {
        qCDebug(SYNC_WORKER_CATEGORY) << formatSyncChange(insertedOld.at(i), insertedNew.at(i));
    }
    for (int i = 0; i < updatedNew.size(); i++) {
        qCDebug(SYNC_WORKER_CATEGORY) << formatSyncChange(updatedOld.at(i), updatedNew.at(i));
    }
}

void DataSyncerWorker::syncDataSynced(QVector<TimeLogSyncData> updatedData, QVector<TimeLogSyncData> removedData)
{
    Q_UNUSED(updatedData)
    Q_UNUSED(removedData)

    if (!m_importState->active() || m_pack) { // sync with pack
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
        emit synced(QPrivateSignal());
    }
}

void DataSyncerWorker::packImported(QDateTime latestMTime)
{
    Q_UNUSED(latestMTime)

    delete m_dbSyncer;
    m_dbSyncer = nullptr;
    delete m_pack;
    m_pack = nullptr;

    qCInfo(SYNC_WORKER_CATEGORY) << "Successfully imported pack" << m_fileList.at(m_currentIndex);

    processCurrentItemImported();
}

void DataSyncerWorker::packExported(QDateTime latestMTime)
{
    delete m_dbSyncer;
    m_dbSyncer = nullptr;
    delete m_pack;
    m_pack = nullptr;

    QDir packDir(m_dir.filePath("pack"));

    if (latestMTime.isNull()) {
        qCInfo(SYNC_WORKER_CATEGORY) << "No new data, leaving old pack" << m_packName;
        if (!packDir.remove("pack.pack")) {
            fail(QString("Fail to remove file %1").arg(packDir.filePath("pack.pack")));
            return;
        }
    } else {
        const QDateTime &mTime = qMax(latestMTime, m_packMTime);
        QString mTimeString = QString("%1").arg(mTime.toMSecsSinceEpoch(), mTimeLength, 10, QChar('0'));
        m_packName = QString("%1-%2.pack").arg(mTimeString).arg(QUuid::createUuid().toString());
        if (!copyFile(packDir.filePath("pack.pack"), m_dir.filePath(m_packName), true, true)) {
            return;
        }
        if (!copyFile(m_dir.filePath(m_packName), QDir(m_syncPath).filePath(m_packName), true, false)) {
            return;
        }
        qCInfo(SYNC_WORKER_CATEGORY) << "Successfully written pack" << m_dir.filePath(m_packName);
    }

    if (removeOldFiles(m_packName)) {
        qCInfo(SYNC_WORKER_CATEGORY) << "Successfully packed";
        emit synced(QPrivateSignal());
    }
}

void DataSyncerWorker::startImport()
{
    m_fileList = AbstractDataInOut::buildFileList(m_dir.filePath("incoming"));
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
    if (!AbstractDataInOut::prepareDir(m_intPath, m_dir)) {
        fail(QString("Fail to prepare directory %1").arg(m_intPath));
        return;
    }

    QStringList fileList = AbstractDataInOut::buildFileList(m_intPath);
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

    QDateTime mFrom = QDateTime::fromMSecsSinceEpoch(mTimeString.isEmpty() ? 0 : mTimeString.toLongLong());

    m_db->getSyncData(mFrom);
}

void DataSyncerWorker::syncFolders()
{
    compareWithDir(m_syncPath);

    qCDebug(SYNC_WORKER_CATEGORY) << "Out files:" << m_outFiles;
    qCDebug(SYNC_WORKER_CATEGORY) << "In files:" << m_inFiles;

    if (!copyFiles(m_dir.path(), m_syncPath, m_outFiles, false)
        || !copyFiles(m_syncPath, m_dir.filePath("incoming"), m_inFiles, false)) {
        return;
    }

    qCInfo(SYNC_WORKER_CATEGORY) << "Folders synced";

    emit foldersSynced(QPrivateSignal());
}

void DataSyncerWorker::packSync()
{
    QStringList fileList = AbstractDataInOut::buildFileList(m_intPath);
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
        m_packMTime = QDateTime::fromMSecsSinceEpoch(packMTimeString.toLongLong());
    } else {
        m_packMTime = QDateTime::fromMSecsSinceEpoch(-1);   // TODO: mtime >= :mBegin
    }

    QDateTime packPeriodStart(maxPackPeriodStart());
    if (m_forcePack) {
        exportPack();
    } else if (m_packMTime < packPeriodStart) {
        m_db->checkHasSyncData(m_packMTime, packPeriodStart.addMonths(1).addMSecs(-1));
    } else {
        emit synced(QPrivateSignal());
    }
}

void DataSyncerWorker::cleanState()
{
    m_fileList.clear();
    m_currentIndex = 0;
    m_outFiles.clear();
    m_inFiles.clear();

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

void DataSyncerWorker::compareWithDir(const QString &path)
{
    QDir dir(path);
    QSet<QString> extEntries =  dir.entryList(QDir::Files).toSet();
    QSet<QString> intEntries = m_dir.entryList(QDir::Files).toSet();

    m_outFiles = QSet<QString>(intEntries).subtract(extEntries);
    m_inFiles = QSet<QString>(extEntries).subtract(intEntries);
}

bool DataSyncerWorker::copyFiles(const QString &from, const QString &to, const QSet<QString> fileList, bool isRemoveSource)
{
    QDir sourceDir(from);
    QDir destinationDir;
    if (!AbstractDataInOut::prepareDir(to, destinationDir)) {
        fail(QString("Fail to prepare directory %1").arg(to));
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
            fail(AbstractDataInOut::formatFileError("Fail to remove file", destinationFile));
            return false;
        }
    }
    if (!(isRemoveSource ? sourceFile.rename(destinationFile.fileName())
                         : sourceFile.copy(destinationFile.fileName()))) {
        fail(AbstractDataInOut::formatFileError(QString("Fail to %1 file to %2 from")
                             .arg(isRemoveSource ? "move" : "copy")
                             .arg(destinationFile.fileName()),
                             sourceFile));
        return false;
    }

    return true;
}

bool DataSyncerWorker::exportFile(const QVector<TimeLogSyncData> &data)
{
    QDir exportDir;
    if (!AbstractDataInOut::prepareDir(m_dir.filePath("export"), exportDir)) {
        fail(QString("Fail to prepare directory %1").arg(m_dir.filePath("export")));
        return false;
    }

    QString fileName = QString("%1-%2.sync").arg(data.last().mTime.toMSecsSinceEpoch(), mTimeLength, 10, QChar('0'))
                                            .arg(QUuid::createUuid().toString());
    QString filePath = exportDir.filePath(fileName);

    qCInfo(SYNC_WORKER_CATEGORY) << "Exporting data to file" << filePath;

    QByteArray fileData;
    QDataStream dataStream(&fileData, QIODevice::WriteOnly);
    dataStream.setVersion(syncFileStreamVersion);
    dataStream << syncFileFormatVersion;

    for (int i = 0; i < data.size(); i++) {
        qCInfo(SYNC_WORKER_CATEGORY) << QString("Exported item %1 of %2").arg(i+1).arg(data.size());
        qCDebug(SYNC_WORKER_CATEGORY) << data.at(i).toString();
        dataStream << data.at(i);
    }

    QFile file(filePath);
    if (file.exists()) {
        qCWarning(SYNC_WORKER_CATEGORY) << "File already exists, overwriting" << filePath;
        return false;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        fail(AbstractDataInOut::formatFileError("Fail to open file", file));
        return false;
    }

    QDataStream fileStream(&file);
    fileStream << syncFileStreamVersion;
    fileStream.setVersion(syncFileStreamVersion);

    fileStream << fileData;
    fileStream << qChecksum(fileData.constData(), fileData.size());

    if (fileStream.status() != QDataStream::Ok || file.error() != QFileDevice::NoError) {
        fail(AbstractDataInOut::formatFileError("Error writing to file", file));
        return false;
    }

    file.close();

    if (!copyFile(filePath, m_dir.filePath(fileName), true, true)) {
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
    QVector<TimeLogSyncData> updatedData, removedData;

    if (!parseFile(path, updatedData, removedData)) {
        return;
    }

    if (!updatedData.isEmpty() || !removedData.isEmpty()) {
        m_db->sync(updatedData, removedData);
    } else {
        syncDataSynced(updatedData, removedData);
    }
}

bool DataSyncerWorker::parseFile(const QString &path, QVector<TimeLogSyncData> &updatedData, QVector<TimeLogSyncData> &removedData) const
{
    QFile file(path);
    if (!file.exists()) {
        fail(QString("File %1 does not exists").arg(path));
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        fail(AbstractDataInOut::formatFileError("Fail to open file", file));
        return false;
    }

    QDataStream fileStream(&file);

    if (fileStream.atEnd()) {
        fail(QString("Invalid file %1, no stream version").arg(path));
        return false;
    }
    qint32 streamVersion;
    fileStream >> streamVersion;
    if (streamVersion > syncFileStreamVersion) {
        fail(QString("Stream format version too new: %1 > %2")
             .arg(streamVersion).arg(syncFileStreamVersion));
        return false;
    }
    fileStream.setVersion(streamVersion);

    if (fileStream.atEnd()) {
        fail(QString("Invalid file %1, no data").arg(path));
        return false;
    }
    QByteArray fileData;
    fileStream >> fileData;
    if (fileStream.status() != QDataStream::Ok || file.error() != QFileDevice::NoError) {
        fail(AbstractDataInOut::formatFileError(QString("Error reading from file, stream status %1")
                                                .arg(fileStream.status()),
                                                file));
        return false;
    }

    if (fileStream.atEnd()) {
        fail(QString("Invalid file %1, no checksum").arg(path));
        return false;
    }
    quint16 checksum;
    fileStream >> checksum;
    if (qChecksum(fileData.constData(), fileData.size()) != checksum) {
        fail(QString("Broken file %1, checksums does not match").arg(path));
        return false;
    }

    QDataStream dataStream(&fileData, QIODevice::ReadOnly);
    dataStream.setVersion(streamVersion);

    if (dataStream.atEnd()) {
        fail(QString("Invalid data in file %1, no format version").arg(path));
        return false;
    }
    qint32 formatVersion;
    dataStream >> formatVersion;
    if (formatVersion != syncFileFormatVersion) {
        fail(QString("Data format version %1 instead of %2")
             .arg(formatVersion).arg(syncFileFormatVersion));
        return false;
    }

    while (!dataStream.atEnd()) {
        TimeLogSyncData entry;
        dataStream >> entry;

        if (entry.isValid()) {
            qCDebug(SYNC_WORKER_CATEGORY) << "Valid entry:" << entry;
            updatedData.append(entry);
        } else if (!entry.uuid.isNull() && entry.mTime.isValid()) {
            qCDebug(SYNC_WORKER_CATEGORY) << "Valid removed entry:" << entry;
            removedData.append(entry);
        } else {
            qCWarning(SYNC_WORKER_CATEGORY) << "Invalid entry:" << entry;
        }
    }

    if (updatedData.isEmpty() && removedData.isEmpty()) {
        qCWarning(SYNC_WORKER_CATEGORY) << "No data in file" << path;
    }

    return true;
}

void DataSyncerWorker::importPack(const QString &path)
{
    m_pack = new TimeLogHistory(this);
    if (!m_pack->init(m_intPath, m_dir.relativeFilePath(path))) {
        fail("Fail to open pack");
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
        if (copyFiles(m_dir.filePath("incoming"), m_dir.path(), m_inFiles, true)) {
            emit imported(QPrivateSignal());
        }
    } else {
        importCurrentItem();
    }
}

void DataSyncerWorker::exportPack()
{
    QDir packDir;
    if (!AbstractDataInOut::prepareDir(m_dir.filePath("pack"), packDir)) {
        fail(QString("Fail to prepare directory %1").arg(m_dir.filePath("pack")));
        return;
    }

    if (!m_packName.isEmpty()) {
        if (!copyFile(m_dir.filePath(m_packName), packDir.filePath("pack.pack"), true, false)) {
            fail(QString("Fail to copy pack file to %1").arg(packDir.filePath("pack.pack")));
            return;
        }
        qCDebug(SYNC_WORKER_CATEGORY) << "Using existing pack file" << m_packName;
    } else {
        qCDebug(SYNC_WORKER_CATEGORY) << "No existing pack file, creating new";
    }

    m_pack = new TimeLogHistory(this);
    if (!m_pack->init(m_intPath, m_dir.relativeFilePath(packDir.filePath("pack.pack")))) {
        fail("Fail to create pack");
        return;
    }
    connect(m_pack, SIGNAL(syncStatsAvailable(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                              QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                              QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)),
            this, SLOT(syncStatsAvailable(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                          QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                          QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)));

    m_dbSyncer = new DBSyncer(m_db, m_pack, this);
    connect(m_dbSyncer, SIGNAL(finished(QDateTime)),
            this, SLOT(packExported(QDateTime)));
    connect(m_dbSyncer, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));

    m_dbSyncer->start(true, maxPackPeriodStart());
}

QString DataSyncerWorker::formatSyncChange(const TimeLogSyncData &oldData, const TimeLogSyncData &newData) const
{
    QStringList result;

    if (newData.isValid()) {
        if (!oldData.uuid.isNull()) {
            result << "[Updated]" << oldData.toString() << "->" << newData.toString();
        } else {
            result << "[New item]" << newData.toString();
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
    QStringList fileList = AbstractDataInOut::buildFileList(m_intPath);
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

        if (!m_dir.remove(fileName)) {
            fail(QString("Fail to remove file %1").arg(filePath));
            return false;
        }
        qCDebug(SYNC_WORKER_CATEGORY) << "Removed" << m_dir.filePath(fileName);
    }

    QDir syncDir(m_syncPath);
    fileList = AbstractDataInOut::buildFileList(m_syncPath);
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
            fail(QString("Fail to remove file %1").arg(filePath));
            return false;
        }
        qCDebug(SYNC_WORKER_CATEGORY) << "Removed" << syncDir.filePath(fileName);
    }

    return true;
}
