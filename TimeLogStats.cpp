#include "TimeLogStats.h"

TimeLogStats::TimeLogStats() :
    durationTime(0),
    category(QString())
{

}

TimeLogStats::TimeLogStats(int durationTime, const QString &category) :
    durationTime(durationTime),
    category(category)
{

}
