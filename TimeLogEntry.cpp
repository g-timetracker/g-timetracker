#include "TimeLogEntry.h"

TimeLogEntry::TimeLogEntry(const QUuid &uuid, const TimeLogData &data) :
    TimeLogData(data),
    uuid(uuid)
{

}
