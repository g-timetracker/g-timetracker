#ifndef TIMELOGENTRY_H
#define TIMELOGENTRY_H

#include <QUuid>
#include <QDateTime>

struct TimeLogEntry
{
public:
    TimeLogEntry();

    QUuid id;
    QDateTime startTime;
    qint64 durationTime;
    QString category;
    QString comment;
    QDateTime mTime;
};

#endif // TIMELOGENTRY_H
