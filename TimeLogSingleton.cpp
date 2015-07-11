#include "TimeLogSingleton.h"

TimeLogSingleton::TimeLogSingleton(QObject *parent) : QObject(parent)
{

}

TimeLogData TimeLogSingleton::createTimeLogData(QDateTime startTime, int durationTime,
                                                QString category, QString comment)
{
    return TimeLogData(startTime, durationTime, category, comment);
}
