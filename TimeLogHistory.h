#ifndef TIMELOGHISTORY_H
#define TIMELOGHISTORY_H

#include <QObject>
#include <QSqlQuery>

#include "TimeLogEntry.h"

class TimeLogHistory : public QObject
{
    Q_OBJECT
public:
    explicit TimeLogHistory(QObject *parent = 0);
    ~TimeLogHistory();

    void insert(const TimeLogEntry &data);
    void remove(const QUuid &uuid);
    void edit(const TimeLogEntry &data);

    QVector<QString> categories(const QDateTime &begin = QDateTime::fromTime_t(0),
                                const QDateTime &end = QDateTime::currentDateTime()) const;
    bool hasHistory(const QDateTime &before = QDateTime::currentDateTime()) const;
    QVector<TimeLogEntry> getHistory(const QDateTime &begin = QDateTime::fromTime_t(0),
                                     const QDateTime &end = QDateTime::currentDateTime()) const;
    QVector<TimeLogEntry> getHistory(const uint limit,
                                     const QDateTime &until = QDateTime::currentDateTime()) const;

private:
    QVector<TimeLogEntry> getHistory(QSqlQuery &query) const;
};

#endif // TIMELOGHISTORY_H
