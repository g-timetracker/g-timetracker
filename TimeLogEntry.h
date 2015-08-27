#ifndef TIMELOGENTRY_H
#define TIMELOGENTRY_H

#include <QUuid>

#include "TimeLogData.h"

struct TimeLogEntry: public TimeLogData
{
public:
    TimeLogEntry(const QUuid &uuid = QUuid(), const TimeLogData &data = TimeLogData());

    QUuid uuid;
};

Q_DECLARE_TYPEINFO(TimeLogEntry, Q_MOVABLE_TYPE);

#endif // TIMELOGENTRY_H
