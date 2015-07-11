#ifndef TIMELOGENTRY_H
#define TIMELOGENTRY_H

#include <QUuid>

#include "TimeLogData.h"

struct TimeLogEntry: public TimeLogData
{
//    Q_GADGET
public:
    TimeLogEntry(TimeLogData data = TimeLogData());

//    qint64 durationTime;
    QUuid id;
    QDateTime mTime;
};

#endif // TIMELOGENTRY_H
