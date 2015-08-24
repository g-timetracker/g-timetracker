#ifndef TIMELOGENTRY_H
#define TIMELOGENTRY_H

#include <QUuid>

#include "TimeLogData.h"

struct TimeLogEntry: public TimeLogData
{
//    Q_GADGET
public:
    TimeLogEntry(const QUuid &uuid = QUuid(), const TimeLogData &data = TimeLogData());

    QUuid uuid;
};

#endif // TIMELOGENTRY_H
