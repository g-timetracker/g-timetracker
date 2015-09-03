#ifndef TIMELOGENTRY_H
#define TIMELOGENTRY_H

#include <QUuid>

#include "TimeLogData.h"

struct TimeLogEntry: public TimeLogData
{
    typedef TimeLogData SUPER;
public:
    TimeLogEntry(const QUuid &uuid = QUuid(), const TimeLogData &data = TimeLogData());

    bool isValid() const;

    QUuid uuid;
};

Q_DECLARE_TYPEINFO(TimeLogEntry, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TimeLogEntry)

#endif // TIMELOGENTRY_H
