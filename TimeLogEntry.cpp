#include "TimeLogEntry.h"

TimeLogEntry::TimeLogEntry() :
    startTime(QDateTime::currentDateTimeUtc()),
    durationTime(0),
    category("Empty category"),
    comment("Test comment"),
    mTime(QDateTime::currentDateTimeUtc())
{

}
