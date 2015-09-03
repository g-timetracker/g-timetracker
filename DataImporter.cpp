#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include <QLoggingCategory>

#include "DataImporter.h"
#include "TimeLogHistory.h"

Q_LOGGING_CATEGORY(DATA_IMPORTER_CATEGORY, "DataImporter", QtInfoMsg)

DataImporter::DataImporter(TimeLogHistory *history) :
    m_db(history),
    m_sep(";")
{

}

DataImporter::~DataImporter()
{

}

bool DataImporter::import(const QString &path) const
{
    return processPath(path);
}

void DataImporter::setSeparator(const QString &sep)
{
    m_sep = sep;
}

bool DataImporter::processPath(const QString &path) const
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        qCCritical(DATA_IMPORTER_CATEGORY) << "Path does not exists" << path;
        return false;
    }

    if (fileInfo.isFile()) {
        return processFile(path);
    } else if (fileInfo.isDir()) {
        return processDirectory(path);
    } else {
        qCCritical(DATA_IMPORTER_CATEGORY) << "Not file or directory" << path;
        return false;
    }
}

bool DataImporter::processDirectory(const QString &path) const
{
    QDir dir(path);

    QStringList entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst);
    foreach (const QString &file, entries) {
        QString filePath = dir.filePath(file);
        if (!processPath(filePath)) {
            return false;
        }
    }

    return true;
}

bool DataImporter::processFile(const QString &path) const
{
    QVector<TimeLogEntry> data = parseFile(path);

    if (data.size()) {
        foreach (const TimeLogEntry &entry, data) {
            m_db->insert(entry);
        }

        qCInfo(DATA_IMPORTER_CATEGORY) << "Successfully imported file" << path;
    }

    return !data.isEmpty();
}

QVector<TimeLogEntry> DataImporter::parseFile(const QString &path) const
{
    QVector<TimeLogEntry> result;

    QFile file(path);
    if (!file.exists()) {
        qCCritical(DATA_IMPORTER_CATEGORY) << "File does not exists" << path;
        return result;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCCritical(DATA_IMPORTER_CATEGORY) << "Fail to open file" << path << file.errorString();
        return result;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        TimeLogEntry entry = parseLine(line);
        if (entry.isValid()) {
            result.append(entry);
        } else {
            qCWarning(DATA_IMPORTER_CATEGORY) << "Invalid entry in file" << path << "line:" << line;
        }
    }

    if (!result.size()) {
        qCCritical(DATA_IMPORTER_CATEGORY) << "No data in file" << path;
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
        qCDebug(DATA_IMPORTER_CATEGORY) << "Entry time" << fields.at(0) << result.startTime.toString();
    }
    if (fields.size() >= 2) {
        result.category = fields.at(1);
        qCDebug(DATA_IMPORTER_CATEGORY) << "Entry category" << result.category;
    }
    if (fields.size() >= 3) {
        result.comment = fields.at(2);
        qCDebug(DATA_IMPORTER_CATEGORY) << "Entry comment" << result.comment;
    }
    if (fields.size() >= 4) {
        result.uuid = QUuid(fields.at(3));
        qCDebug(DATA_IMPORTER_CATEGORY) << "Entry uuid" << result.uuid;
    } else {
        result.uuid = QUuid::createUuid();
    }

    return result;
}
