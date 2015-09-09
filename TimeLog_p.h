#ifndef TIMELOG_P_H
#define TIMELOG_P_H

#include "TimeLog.h"

class TimeLogSingleton: public TimeLog
{
    Q_OBJECT
public:
    explicit TimeLogSingleton(QObject *parent = 0) : TimeLog(parent) { }
};

#endif // TIMELOG_P_H
