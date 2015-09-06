#ifndef DATAIMPORTER_H
#define DATAIMPORTER_H

#include "TimeLogEntry.h"

class TimeLogHistory;

class DataImporter
{
public:
    explicit DataImporter(TimeLogHistory *history);
    ~DataImporter();

    bool import(const QString &path) const;
    void setSeparator(const QString &sep);

private:
    bool processPath(const QString &path) const;
    bool processDirectory(const QString &path) const;
    bool processFile(const QString &path) const;
    QVector<TimeLogEntry> parseFile(const QString &path) const;
    TimeLogEntry parseLine(const QString &line) const;

    TimeLogHistory *m_db;
    QString m_sep;
};

#endif // DATAIMPORTER_H