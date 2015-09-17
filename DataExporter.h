#ifndef DATAEXPORTER_H
#define DATAEXPORTER_H

#include <QObject>
#include <QDir>

#include "TimeLogEntry.h"

class TimeLogHistory;

class DataExporter : public QObject
{
    Q_OBJECT
public:
    explicit DataExporter(QObject *parent = 0);

    void exportData(const QString &path);
    void setSeparator(const QString &sep);

private slots:
    void startExport(const QString &path);
    void historyError(const QString &errorText);
    void historyDataAvailable(QDateTime from, QVector<TimeLogEntry> data);
    void historyDataAvailable(QVector<TimeLogEntry> data, QDateTime until);

private:
    bool prepareDir(const QString &path);
    void exportCurrentDate();
    bool exportDay(QVector<TimeLogEntry> data);

    TimeLogHistory *m_db;
    QString m_sep;
    QDir m_dir;
    QDate m_currentDate;
    QDate m_lastDate;
};

#endif // DATAEXPORTER_H
