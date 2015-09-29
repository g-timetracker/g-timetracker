#include <QCoreApplication>
#include <QStandardPaths>
#include <QDataStream>
#include <QRegularExpression>

#include <QLoggingCategory>

#include "DataSyncer.h"
#include "TimeLogHistory.h"

Q_LOGGING_CATEGORY(DATA_SYNC_CATEGORY, "DataSync", QtInfoMsg)

const qint32 syncFileFormatVersion = 1;
const qint32 syncFileStreamVersion = QDataStream::Qt_5_6;

const int mTimeLength = QString::number(std::numeric_limits<qint64>::max()).length();
const QString fileNamePattern = QString("^(?<mTime>\\d{%1})-\\{[\\w-]+\\}$").arg(mTimeLength);
const QRegularExpression fileNameRegexp(fileNamePattern);

DataSyncer::DataSyncer(QObject *parent) :
    AbstractDataInOut(parent)
{
    connect(m_db, SIGNAL(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)),
            this, SLOT(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)));
    connect(m_db, SIGNAL(dataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)),
            this, SLOT(syncDataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)));
}

void DataSyncer::startIO(const QString &path)
{
    startSync(path);
}

void DataSyncer::historyError(const QString &errorText)
{
    qCCritical(DATA_SYNC_CATEGORY) << "Fail to get data from db:" << errorText;
}

void DataSyncer::syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until)
{
    Q_UNUSED(until)

    if (!data.isEmpty()) {
        if (!exportData(data)) {
            QCoreApplication::exit(EXIT_FAILURE);
            return;
        }
    } else {
        qCInfo(DATA_SYNC_CATEGORY) << "No data to export";
    }

    syncFolders(m_syncPath);
}

void DataSyncer::syncDataSynced(QVector<TimeLogSyncData> updatedData, QVector<TimeLogSyncData> removedData)
{
    Q_UNUSED(updatedData)
    Q_UNUSED(removedData)

    qCInfo(DATA_SYNC_CATEGORY) << "Successfully imported file" << m_fileList.at(m_currentIndex);

    ++m_currentIndex;
    if (m_currentIndex == m_fileList.size()) {
        if (!copyFiles(m_dir.filePath("incoming"), m_dir.path(), m_inFiles, true)) {
            QCoreApplication::exit(EXIT_FAILURE);
        } else {
            QCoreApplication::quit();
        }
    } else {
        importCurrentFile();
    }
}

void DataSyncer::startSync(const QString &path)
{
    m_syncPath = path;

    QString intPath(QString("%1/sync")
                    .arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation)));

    startExport(intPath);
}

void DataSyncer::startImport(const QString &path)
{
    m_fileList.clear();
    if (!buildFileList(path, m_fileList)) {
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    if (m_fileList.isEmpty()) {
        qCInfo(DATA_SYNC_CATEGORY) << "No files to import";
        QCoreApplication::quit();
        return;
    }

    std::sort(m_fileList.begin(), m_fileList.end());
    m_currentIndex = 0;
    importCurrentFile();
}

void DataSyncer::startExport(const QString &path)
{
    if (!prepareDir(path, m_dir)) {
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    QStringList fileList;
    if (!buildFileList(path, fileList)) {
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    QString mTimeString;
    std::sort(fileList.begin(), fileList.end());
    for (QList<QString>::const_reverse_iterator it = fileList.crbegin(); it != fileList.crend(); it++) {
        QString fileName = QFileInfo(*it).fileName();
        QRegularExpressionMatch match = fileNameRegexp.match(fileName);
        if (!match.hasMatch()) {
            qCInfo(DATA_SYNC_CATEGORY) << "Skipping file not matching pattern" << fileName;
            continue;
        }

        mTimeString = match.captured("mTime");
        break;
    }

    QDateTime mFrom = QDateTime::fromMSecsSinceEpoch(mTimeString.isEmpty() ? 0 : mTimeString.toLongLong());

    m_db->getSyncData(mFrom);
}

void DataSyncer::syncFolders(const QString &path)
{
    compareWithDir(path);

    qCDebug(DATA_SYNC_CATEGORY) << "Out files:" << m_outFiles;
    qCDebug(DATA_SYNC_CATEGORY) << "In files:" << m_inFiles;

    if (!copyFiles(m_dir.path(), path, m_outFiles, false)
        || !copyFiles(path, m_dir.filePath("incoming"), m_inFiles, false)) {
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    qCInfo(DATA_SYNC_CATEGORY) << "Folders synced";

    startImport(m_dir.filePath("incoming"));
}

void DataSyncer::compareWithDir(const QString &path)
{
    QDir dir(path);
    QSet<QString> extEntries =  dir.entryList(QDir::Files).toSet();
    QSet<QString> intEntries = m_dir.entryList(QDir::Files).toSet();

    m_outFiles = QSet<QString>(intEntries).subtract(extEntries);
    m_inFiles = QSet<QString>(extEntries).subtract(intEntries);
}

bool DataSyncer::copyFiles(const QString &from, const QString &to, const QSet<QString> fileList, bool isRemoveSource)
{
    QDir sourceDir(from);
    QDir destinationDir;
    if (!prepareDir(to, destinationDir)) {
        return false;
    }

    foreach (const QString fileName, fileList) {
        if (!copyFile(sourceDir.filePath(fileName), destinationDir.filePath(fileName), true, isRemoveSource)) {
            return false;
        }
    }

    return true;
}

bool DataSyncer::copyFile(const QString &source, const QString &destination, bool isOverwrite, bool isRemoveSource) const
{
    QFile sourceFile(source);
    QFile destinationFile(destination);
    if (destinationFile.exists() && isOverwrite) {
        qCInfo(DATA_SYNC_CATEGORY) << QString("File %1 already exists, removing")
                                      .arg(destinationFile.fileName());
        if (!destinationFile.remove()) {
            qCCritical(DATA_SYNC_CATEGORY) << formatFileError("Fail to remove file", destinationFile);
            return false;
        }
    }
    if (!(isRemoveSource ? sourceFile.rename(destinationFile.fileName())
                         : sourceFile.copy(destinationFile.fileName()))) {
        qCCritical(DATA_SYNC_CATEGORY) << formatFileError(QString("Fail to %1 file to %2 from")
                                                          .arg(isRemoveSource ? "move" : "copy")
                                                          .arg(destinationFile.fileName()),
                                                          sourceFile);
        return false;
    }

    return true;
}

bool DataSyncer::exportData(const QVector<TimeLogSyncData> &data)
{
    QDir exportDir;
    if (!prepareDir(m_dir.filePath("export"), exportDir)) {
        return false;
    }

    QString fileName = QString("%1-%2").arg(data.last().mTime.toMSecsSinceEpoch(), mTimeLength, 10, QChar('0'))
                                       .arg(QUuid::createUuid().toString());
    QString filePath = exportDir.filePath(fileName);
    QFile file(filePath);
    if (file.exists()) {
        qCCritical(DATA_SYNC_CATEGORY) << "File already exists, overwriting" << filePath;
        return false;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCCritical(DATA_SYNC_CATEGORY) << formatFileError("Fail to open file", file);
        return false;
    }

    qCInfo(DATA_SYNC_CATEGORY) << "Exporting data to file" << filePath;

    QDataStream stream(&file);
    stream << syncFileFormatVersion;
    stream << syncFileStreamVersion;
    stream.setVersion(syncFileStreamVersion);

    for (int i = 0; i < data.size(); i++) {
        qCDebug(DATA_SYNC_CATEGORY) << "Exporting entry:" << data.at(i);
        stream << data.at(i);
        qCInfo(DATA_SYNC_CATEGORY) << QString("Exported item %1 of %2").arg(i+1).arg(data.size());
    }

    if (stream.status() != QDataStream::Ok || file.error() != QFileDevice::NoError) {
        qCCritical(DATA_SYNC_CATEGORY) << formatFileError("Error writing to file", file);
        return false;
    }

    file.close();

    if (!copyFile(filePath, m_dir.filePath(fileName), true, true)) {
        return false;
    }

    qCInfo(DATA_SYNC_CATEGORY) << "All data successfully exported";

    return true;
}

void DataSyncer::importCurrentFile()
{
    qCInfo(DATA_SYNC_CATEGORY) << QString("Importing file %1 of %2")
                                  .arg(m_currentIndex+1).arg(m_fileList.size());
    importFile(m_fileList.at(m_currentIndex));
}

void DataSyncer::importFile(const QString &path)
{
    QVector<TimeLogSyncData> updatedData, removedData;

    if (!parseFile(path, updatedData, removedData)) {
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    if (!updatedData.isEmpty() || !removedData.isEmpty()) {
        m_db->sync(updatedData, removedData);
    } else {
        syncDataSynced(updatedData, removedData);
    }
}

bool DataSyncer::parseFile(const QString &path, QVector<TimeLogSyncData> &updatedData, QVector<TimeLogSyncData> &removedData) const
{
    QFile file(path);
    if (!file.exists()) {
        qCCritical(DATA_SYNC_CATEGORY) << "File does not exists" << path;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCCritical(DATA_SYNC_CATEGORY) << formatFileError("Fail to open file", file);
        return false;
    }

    QDataStream stream(&file);
    if (stream.atEnd()) {
        qCCritical(DATA_SYNC_CATEGORY) << "Invalid file, no format version" << path;
        return false;
    }
    qint32 formatVersion;
    stream >> formatVersion;
    if (formatVersion != syncFileFormatVersion) {
        qCCritical(DATA_SYNC_CATEGORY) << QString("File format version %1 instead of %2")
                                          .arg(formatVersion).arg(syncFileFormatVersion);
        return false;
    }
    if (stream.atEnd()) {
        qCCritical(DATA_SYNC_CATEGORY) << "Invalid file, no stream version" << path;
        return false;
    }
    qint32 streamVersion;
    stream >> streamVersion;
    if (streamVersion > syncFileStreamVersion) {
        qCCritical(DATA_SYNC_CATEGORY) << QString("Stream format version too new: %1 > %2")
                                          .arg(streamVersion).arg(syncFileStreamVersion);
        return false;
    }
    stream.setVersion(streamVersion);

    while (!stream.atEnd()) {
        TimeLogSyncData entry;
        stream >> entry;

            if (stream.status() != QDataStream::Ok || file.error() != QFileDevice::NoError) {
                qCCritical(DATA_SYNC_CATEGORY) << formatFileError(QString("Error reading from file, stream status %1")
                                                                  .arg(stream.status()), file);
                break;
            }

        if (entry.isValid()) {
            qCDebug(DATA_SYNC_CATEGORY) << "Valid entry:" << entry;
            updatedData.append(entry);
        } else if (!entry.uuid.isNull() && entry.mTime.isValid()) {
            qCDebug(DATA_SYNC_CATEGORY) << "Valid removed entry:" << entry;
            removedData.append(entry);
        } else {
            qCWarning(DATA_SYNC_CATEGORY) << "Invalid entry:" << entry;
        }
    }

    if (updatedData.isEmpty() && removedData.isEmpty()) {
        qCWarning(DATA_SYNC_CATEGORY) << "No data in file" << path;
    }

    return true;
}
