#ifndef DATAIMPORTER_H
#define DATAIMPORTER_H

#include "TimeLogEntry.h"

class TimeLogHistory;

class DataImporter
{
public:
    explicit DataImporter();
    ~DataImporter();

    bool import(const QString &path);
    void setSeparator(const QString &sep);

private:
    bool processPath(const QString &path);
    bool processDirectory(const QString &path);
    bool importFile(const QString &path) const;
    QVector<TimeLogEntry> parseFile(const QString &path) const;
    TimeLogEntry parseLine(const QString &line) const;

    TimeLogHistory *m_db;
    QString m_sep;
    QStringList m_fileList;
};

#endif // DATAIMPORTER_H
