#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QCoreApplication>

#include "DataImporter.h"
#include "TimeLogHistory.h"

#define fail(message) \
    do {    \
        qCCritical(DATA_IO_CATEGORY) << message;    \
        QCoreApplication::exit(EXIT_FAILURE);   \
    } while (0)

DataImporter::DataImporter(QObject *parent) :
    AbstractDataInOut(parent)
{
    connect(m_db, SIGNAL(dataInserted(QVector<TimeLogEntry>)),
            this, SLOT(historyDataInserted(QVector<TimeLogEntry>)));
}

void DataImporter::startIO(const QString &path)
{
    m_fileList = buildFileList(path);
    if (m_fileList.isEmpty()) {
        qCInfo(DATA_IO_CATEGORY) << "No files to import";
        QCoreApplication::quit();
        return;
    }

    m_currentIndex = 0;
    importCurrentFile();
}

void DataImporter::historyError(const QString &errorText)
{
    fail(QString("DB error while importing file %1: %2").arg(m_fileList.at(m_currentIndex)).arg(errorText));
}

void DataImporter::historyDataInserted(QVector<TimeLogEntry> data)
{
    Q_UNUSED(data)

    qCInfo(DATA_IO_CATEGORY) << "Successfully imported file" << m_fileList.at(m_currentIndex);

    ++m_currentIndex;
    if (m_currentIndex == m_fileList.size()) {
        QCoreApplication::quit();
    } else {
        importCurrentFile();
    }
}

void DataImporter::importCurrentFile()
{
    qCInfo(DATA_IO_CATEGORY) << QString("Importing file %1 of %2")
                                .arg(m_currentIndex+1).arg(m_fileList.size());
    importFile(m_fileList.at(m_currentIndex));
}

void DataImporter::importFile(const QString &path)
{
    QVector<TimeLogEntry> data = parseFile(path);

    if (data.size()) {
        m_db->insert(data);
    } else {
        historyDataInserted(data);
    }
}

QVector<TimeLogEntry> DataImporter::parseFile(const QString &path) const
{
    QVector<TimeLogEntry> result;

    QFile file(path);
    if (!file.exists()) {
        fail(QString("File %1 does not exists").arg(path));
        return result;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fail(formatFileError("Fail to open file", file));
        return result;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        TimeLogEntry entry = parseLine(line);
        if (entry.isValid()) {
            result.append(entry);
        } else {
            qCWarning(DATA_IO_CATEGORY) << "Invalid entry in file" << path << "line:" << line;
        }
    }

    if (!result.size()) {
        qCWarning(DATA_IO_CATEGORY) << "No data in file" << path;
    }

    return result;
}

TimeLogEntry DataImporter::parseLine(const QString &line) const
{
    TimeLogEntry result;

    QStringList fields = line.split(m_sep, QString::KeepEmptyParts);
    if (fields.isEmpty()) {
        return result;
    }

    if (fields.size() >= 1) {
        result.startTime = QDateTime::fromString(fields.at(0), Qt::ISODate);
        qCDebug(DATA_IO_CATEGORY) << "Entry time" << fields.at(0) << result.startTime.toTime_t();
    }
    if (fields.size() >= 2) {
        result.category = fields.at(1);
        qCDebug(DATA_IO_CATEGORY) << "Entry category" << result.category;
    }
    if (fields.size() >= 3) {
        result.comment = fields.at(2);
        qCDebug(DATA_IO_CATEGORY) << "Entry comment" << result.comment;
    }
    if (fields.size() >= 4) {
        result.uuid = QUuid(fields.at(3));
        qCDebug(DATA_IO_CATEGORY) << "Entry uuid" << result.uuid;
    } else {
        result.uuid = QUuid::createUuid();
    }

    return result;
}
