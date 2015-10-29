#include "TimeLogEntry.h"

TimeLogEntry::TimeLogEntry(const QUuid &uuid, const TimeLogData &data) :
    TimeLogData(data),
    uuid(uuid),
    precedingStart()
{

}

bool TimeLogEntry::isValid() const
{
    return (SUPER::isValid() && !uuid.isNull());
}
