#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include <QDebug>

#include "DataImporter.h"
#include "TimeLogHistory.h"

DataImporter::DataImporter() :
    m_db(new TimeLogHistory),
    m_sep(";")
{

}

DataImporter::~DataImporter()
{
    delete m_db;
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
        qCritical() << "Path does not exists" << path;
        return false;
    }

    if (fileInfo.isFile()) {
        return processFile(path);
    } else if (fileInfo.isDir()) {
        return processDirectory(path);
    } else {
        qCritical() << "Not file or directory" << path;
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

        qInfo() << "Successfully imported file" << path;
    }

    return !data.isEmpty();
}

QVector<TimeLogEntry> DataImporter::parseFile(const QString &path) const
{
    QVector<TimeLogEntry> result;

    QFile file(path);
    if (!file.exists()) {
        qCritical() << "File does not exists" << path;
        return result;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Fail to open file" << path << file.errorString();
        return result;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        TimeLogEntry entry = parseLine(line);
        if (entry.isValid()) {
            result.append(entry);
        } else {
            qWarning() << "Invalid entry in file" << path << "line:" << line;
        }
    }

    if (!result.size()) {
        qCritical() << "No data in file" << path;
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
        qDebug() << "Entry time" << fields.at(0) << result.startTime.toString();
    }
    if (fields.size() >= 2) {
        result.category = fields.at(1);
        qDebug() << "Entry category" << result.category;
    }
    if (fields.size() >= 3) {
        result.comment = fields.at(2);
        qDebug() << "Entry comment" << result.comment;
    }
    if (fields.size() >= 4) {
        result.uuid = QUuid(fields.at(3));
        qDebug() << "Entry uuid" << result.uuid;
    } else {
        result.uuid = QUuid::createUuid();
    }

    return result;
}
