#ifndef DATAEXPORTER_H
#define DATAEXPORTER_H

#include "AbstractDataInOut.h"
#include "TimeLogEntry.h"
#include "TimeLogCategory.h"

class DataExporter : public AbstractDataInOut
{
    Q_OBJECT
public:
    explicit DataExporter(TimeLogHistory *db, QObject *parent = 0);

protected slots:
    virtual void startIO(const QString &path);
    virtual void historyError(const QString &errorText);

private slots:
    void historyRequestCompleted(QVector<TimeLogEntry> data, qlonglong id);
    void storedCategoriesAvailable(QVector<TimeLogCategory> data);

private:
    void exportCurrentDate(qlonglong id);
    bool exportDay(const QVector<TimeLogEntry> &data);
    bool exportCategories(const QVector<TimeLogCategory> &data);

    QDir m_dir;
    QDate m_currentDate;
    QDate m_lastDate;
};

#endif // DATAEXPORTER_H
