#include "TimeLogData.h"

TimeLogData::TimeLogData() :
    startTime(QDateTime::currentDateTimeUtc()),
    durationTime(0),
    category("Empty category"),
    comment("Test comment")
{

}

TimeLogData::TimeLogData(QDateTime startTime, int durationTime, QString category, QString comment) :
    startTime(startTime),
    durationTime(durationTime),
    category(category),
    comment(comment)
{

}
