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

    bool init();

public slots:
    void insert(const TimeLogEntry &data);
    void remove(const QUuid &uuid);
    void edit(const TimeLogEntry &data);

public:
    QVector<QString> categories(const QDateTime &begin = QDateTime::fromTime_t(0),
                                const QDateTime &end = QDateTime::currentDateTime()) const;
    bool hasHistory(const QDateTime &before = QDateTime::currentDateTime()) const;
    QVector<TimeLogEntry> getHistory(const QDateTime &begin = QDateTime::fromTime_t(0),
                                     const QDateTime &end = QDateTime::currentDateTime(),
                                     const QString &category = QString()) const;
    QVector<TimeLogEntry> getHistory(const uint limit,
                                     const QDateTime &until = QDateTime::currentDateTime()) const;

signals:
    void error(const QString &errorText) const;

private:
    bool m_isInitialized;

    QVector<TimeLogEntry> getHistory(QSqlQuery &query) const;
};

#endif // TIMELOGHISTORY_H
