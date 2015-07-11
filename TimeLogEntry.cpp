#include "TimeLogEntry.h"

TimeLogEntry::TimeLogEntry(TimeLogData data) :
    TimeLogData(data),
    mTime(QDateTime::currentDateTimeUtc())
{

}
