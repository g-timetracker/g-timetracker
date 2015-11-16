#include "TimeLogData.h"

TimeLogData::TimeLogData() :
    startTime(QDateTime()),
    category(QString()),
    comment(QString())
{

}

TimeLogData::TimeLogData(QDateTime startTime, QString category, QString comment) :
    startTime(startTime),
    category(category),
    comment(comment)
{

}

bool TimeLogData::isValid() const
{
    return (startTime.isValid() && !category.isEmpty());
}
