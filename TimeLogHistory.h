#ifndef TIMELOGHISTORY_H
#define TIMELOGHISTORY_H

#include <QObject>

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
    QVector<TimeLogEntry> getHistory() const;

signals:

public slots:
};

#endif // TIMELOGHISTORY_H
