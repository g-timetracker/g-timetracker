#include "TimeLogData.h"

TimeLogData::TimeLogData() :
    startTime(QDateTime()),
    durationTime(0),
    category(QString()),
    comment(QString())
{

}

TimeLogData::TimeLogData(QDateTime startTime, int durationTime, QString category, QString comment) :
    startTime(startTime),
    durationTime(durationTime),
    category(category),
    comment(comment)
{

}

bool TimeLogData::isValid() const
{
    return startTime.isValid();
}
