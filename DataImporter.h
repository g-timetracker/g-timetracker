#ifndef DATAIMPORTER_H
#define DATAIMPORTER_H

#include "TimeLogEntry.h"

class TimeLogHistory;

class DataImporter
{
public:
    DataImporter();
    ~DataImporter();

    bool import(const QString &path) const;

private:
    bool processPath(const QString &path) const;
    bool processDirectory(const QString &path) const;
    bool processFile(const QString &path) const;
    QVector<TimeLogEntry> parseFile(const QString &path) const;
    TimeLogEntry parseLine(const QString &line) const;

    TimeLogHistory *m_db;
};

#endif // DATAIMPORTER_H
