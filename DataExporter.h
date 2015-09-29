#ifndef DATAEXPORTER_H
#define DATAEXPORTER_H

#include "AbstractDataInOut.h"
#include "TimeLogEntry.h"

class DataExporter : public AbstractDataInOut
{
    Q_OBJECT
public:
    explicit DataExporter(QObject *parent = 0);

protected slots:
    virtual void startIO(const QString &path);
    virtual void historyError(const QString &errorText);

private slots:
    void historyDataAvailable(QDateTime from, QVector<TimeLogEntry> data);
    void historyDataAvailable(QVector<TimeLogEntry> data, QDateTime until);

private:
    void exportCurrentDate();
    bool exportDay(QVector<TimeLogEntry> data);

    QDir m_dir;
    QDate m_currentDate;
    QDate m_lastDate;
};

#endif // DATAEXPORTER_H
