#include <QFile>
#include <QTextStream>

#include <QDebug>

#include "DataImporter.h"
#include "TimeLogHistory.h"

DataImporter::DataImporter()
{

}

bool DataImporter::importFile(const QString &path) const
{
    TimeLogHistory db;

    QFile file(path);
    if (!file.exists()) {
        qCritical() << "File does not exists" << path;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Fail to open file" << path << file.errorString();
        return false;
    }

    QVector<TimeLogEntry> data;

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        TimeLogEntry entry = parseLine(line);
        if (entry.isValid()) {
            data.append(entry);
        } else {
            qWarning() << "Invalid entry in file" << path << "line:" << line;
        }
    }

    if (!data.size()) {
        qCritical() << "No data in file" << path;
        return false;
    }

    foreach (const TimeLogEntry &entry, data) {
        db.insert(entry);
    }

    qInfo() << "Successfully imported file" << path;

    return true;
}

TimeLogEntry DataImporter::parseLine(const QString &line) const
{
    TimeLogEntry result;

    QStringList fields = line.split(';', QString::KeepEmptyParts);
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
