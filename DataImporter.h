#ifndef DATAIMPORTER_H
#define DATAIMPORTER_H

#include <QObject>

#include "TimeLogEntry.h"

class TimeLogHistory;

class DataImporter: public QObject
{
    Q_OBJECT
public:
    explicit DataImporter(QObject *parent = 0);

    void import(const QString &path);
    void setSeparator(const QString &sep);

private slots:
    void startImport(const QString &path);
    void historyError(const QString &errorText);
    void historyDataInserted(QVector<TimeLogEntry> data);

private:
    bool processPath(const QString &path);
    bool processDirectory(const QString &path);
    void importCurrentFile();
    void importFile(const QString &path);
    QVector<TimeLogEntry> parseFile(const QString &path) const;
    TimeLogEntry parseLine(const QString &line) const;

    TimeLogHistory *m_db;
    QString m_sep;
    QStringList m_fileList;
    int m_currentIndex;
};

#endif // DATAIMPORTER_H
