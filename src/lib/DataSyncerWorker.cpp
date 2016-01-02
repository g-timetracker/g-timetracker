#include <QCoreApplication>
#include <QStandardPaths>
#include <QDataStream>
#include <QRegularExpression>
#include <QStateMachine>
#include <QFinalState>

#include <QLoggingCategory>

#include "DataSyncerWorker.h"
#include "TimeLogHistory.h"

#define fail(message) \
    do {    \
        qCCritical(SYNC_WORKER_CATEGORY) << message;    \
        emit error(message);   \
    } while (0)

Q_LOGGING_CATEGORY(SYNC_WORKER_CATEGORY, "DataSyncerWorker", QtInfoMsg)

const qint32 syncFileFormatVersion = 1;
const qint32 syncFileStreamVersion = QDataStream::Qt_5_6;

const int mTimeLength = QString::number(std::numeric_limits<qint64>::max()).length();
const QString fileNamePattern = QString("^(?<mTime>\\d{%1})-\\{[\\w-]+\\}$").arg(mTimeLength);
const QRegularExpression fileNameRegexp(fileNamePattern);

DataSyncerWorker::DataSyncerWorker(TimeLogHistory *db, QObject *parent) :
    AbstractDataInOut(db, parent),
    m_isInitialized(false),
    m_sm(new QStateMachine(this))
{
    QState *exportState = new QState();
    QState *syncFoldersState = new QState();
    QState *importState = new QState();
    QFinalState *finalState = new QFinalState();

    exportState->addTransition(this, SIGNAL(exported()), syncFoldersState);
    syncFoldersState->addTransition(this, SIGNAL(foldersSynced()), importState);
    importState->addTransition(this, SIGNAL(synced()), finalState);

    m_sm->addState(exportState);
    m_sm->addState(syncFoldersState);
    m_sm->addState(importState);
    m_sm->addState(finalState);
    m_sm->setInitialState(exportState);

    connect(this, SIGNAL(started()), m_sm, SLOT(start()));
    connect(this, SIGNAL(error(QString)), m_sm, SLOT(stop()));
    connect(this, SIGNAL(started()), SLOT(startExport()), Qt::QueuedConnection);
    connect(this, SIGNAL(exported()), SLOT(syncFolders()), Qt::QueuedConnection);
    connect(this, SIGNAL(foldersSynced()), SLOT(startImport()), Qt::QueuedConnection);

    connect(m_db, SIGNAL(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)),
            this, SLOT(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)));
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

void DataSyncerWorker::startIO(const QString &path)
{
    Q_ASSERT(m_isInitialized);

    if (m_sm->isRunning()) {
        qCWarning(SYNC_WORKER_CATEGORY) << "Sync already running";
        return;
    } else if (path.isEmpty()) {
        fail("Empty sync path");
        return;
    }

    m_syncPath = path;
    qCInfo(SYNC_WORKER_CATEGORY) << "Syncing with folder" << m_syncPath;

    emit started(QPrivateSignal());
}

void DataSyncerWorker::historyError(const QString &errorText)
{
    Q_UNUSED(errorText);

    m_sm->stop();
}

void DataSyncerWorker::syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until)
{
    Q_UNUSED(until)

    if (!data.isEmpty()) {
        if (!exportData(data)) {
            return;
        }
    } else {
        qCInfo(SYNC_WORKER_CATEGORY) << "No data to export";
    }

    emit exported(QPrivateSignal());
}

void DataSyncerWorker::syncStatsAvailable(QVector<TimeLogSyncData> removedOld, QVector<TimeLogSyncData> removedNew, QVector<TimeLogSyncData> insertedOld, QVector<TimeLogSyncData> insertedNew, QVector<TimeLogSyncData> updatedOld, QVector<TimeLogSyncData> updatedNew) const
{
    qCDebug(SYNC_WORKER_CATEGORY) << "Import details:";
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

    qCInfo(SYNC_WORKER_CATEGORY) << "Successfully imported file" << m_fileList.at(m_currentIndex);

    ++m_currentIndex;
    if (m_currentIndex == m_fileList.size()) {
        if (copyFiles(m_dir.filePath("incoming"), m_dir.path(), m_inFiles, true)) {
            emit synced(QPrivateSignal());
        }
    } else {
        importCurrentFile();
    }
}

void DataSyncerWorker::startImport()
{
    m_fileList = buildFileList(m_dir.filePath("incoming"));
    if (m_fileList.isEmpty()) {
        qCInfo(SYNC_WORKER_CATEGORY) << "No files to import";
        emit synced(QPrivateSignal());
        return;
    }

    std::sort(m_fileList.begin(), m_fileList.end());
    std::reverse(m_fileList.begin(), m_fileList.end());
    m_currentIndex = 0;
    importCurrentFile();
}

void DataSyncerWorker::startExport()
{
    if (!prepareDir(m_intPath, m_dir)) {
        fail(QString("Fail to prepare directory %1").arg(m_intPath));
        return;
    }

    QStringList fileList = buildFileList(m_intPath);
    QString mTimeString;
    std::sort(fileList.begin(), fileList.end());
    for (QList<QString>::const_reverse_iterator it = fileList.crbegin(); it != fileList.crend(); it++) {
        QString fileName = QFileInfo(*it).fileName();
        QRegularExpressionMatch match = fileNameRegexp.match(fileName);
        if (!match.hasMatch()) {
            qCInfo(SYNC_WORKER_CATEGORY) << "Skipping file not matching pattern" << fileName;
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
    if (!prepareDir(to, destinationDir)) {
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
            fail(formatFileError("Fail to remove file", destinationFile));
            return false;
        }
    }
    if (!(isRemoveSource ? sourceFile.rename(destinationFile.fileName())
                         : sourceFile.copy(destinationFile.fileName()))) {
        fail(formatFileError(QString("Fail to %1 file to %2 from")
                             .arg(isRemoveSource ? "move" : "copy")
                             .arg(destinationFile.fileName()),
                             sourceFile));
        return false;
    }

    return true;
}

bool DataSyncerWorker::exportData(const QVector<TimeLogSyncData> &data)
{
    QDir exportDir;
    if (!prepareDir(m_dir.filePath("export"), exportDir)) {
        fail(QString("Fail to prepare directory %1").arg(m_dir.filePath("export")));
        return false;
    }

    QString fileName = QString("%1-%2").arg(data.last().mTime.toMSecsSinceEpoch(), mTimeLength, 10, QChar('0'))
                                       .arg(QUuid::createUuid().toString());
    QString filePath = exportDir.filePath(fileName);
    QFile file(filePath);
    if (file.exists()) {
        qCWarning(SYNC_WORKER_CATEGORY) << "File already exists, overwriting" << filePath;
        return false;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        fail(formatFileError("Fail to open file", file));
        return false;
    }

    qCInfo(SYNC_WORKER_CATEGORY) << "Exporting data to file" << filePath;

    QDataStream stream(&file);
    stream << syncFileFormatVersion;
    stream << syncFileStreamVersion;
    stream.setVersion(syncFileStreamVersion);

    for (int i = 0; i < data.size(); i++) {
        qCInfo(SYNC_WORKER_CATEGORY) << QString("Exported item %1 of %2").arg(i+1).arg(data.size());
        qCDebug(SYNC_WORKER_CATEGORY) << data.at(i).toString();
        stream << data.at(i);
    }

    if (stream.status() != QDataStream::Ok || file.error() != QFileDevice::NoError) {
        fail(formatFileError("Error writing to file", file));
        return false;
    }

    file.close();

    if (!copyFile(filePath, m_dir.filePath(fileName), true, true)) {
        return false;
    }

    qCInfo(SYNC_WORKER_CATEGORY) << "All data successfully exported";

    return true;
}

void DataSyncerWorker::importCurrentFile()
{
    qCInfo(SYNC_WORKER_CATEGORY) << QString("Importing file %1 of %2")
                                    .arg(m_currentIndex+1).arg(m_fileList.size());
    importFile(m_fileList.at(m_currentIndex));
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
        fail(formatFileError("Fail to open file", file));
        return false;
    }

    QDataStream stream(&file);
    if (stream.atEnd()) {
        fail(QString("Invalid file %1, no format version").arg(path));
        return false;
    }
    qint32 formatVersion;
    stream >> formatVersion;
    if (formatVersion != syncFileFormatVersion) {
        fail(QString("File format version %1 instead of %2")
             .arg(formatVersion).arg(syncFileFormatVersion));
        return false;
    }
    if (stream.atEnd()) {
        fail(QString("Invalid file %1, no stream version").arg(path));
        return false;
    }
    qint32 streamVersion;
    stream >> streamVersion;
    if (streamVersion > syncFileStreamVersion) {
        fail(QString("Stream format version too new: %1 > %2")
             .arg(streamVersion).arg(syncFileStreamVersion));
        return false;
    }
    stream.setVersion(streamVersion);

    while (!stream.atEnd()) {
        TimeLogSyncData entry;
        stream >> entry;

            if (stream.status() != QDataStream::Ok || file.error() != QFileDevice::NoError) {
                fail(formatFileError(QString("Error reading from file, stream status %1")
                                     .arg(stream.status()),
                                     file));
                break;
            }

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
