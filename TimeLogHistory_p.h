#ifndef TIMELOGHISTORY_P_H
#define TIMELOGHISTORY_P_H

#include "TimeLogHistory.h"

class TimeLogHistorySingleton: public TimeLogHistory
{
    Q_OBJECT
public:
    explicit TimeLogHistorySingleton(QObject *parent = 0) : TimeLogHistory(parent) { }
};

#endif // TIMELOGHISTORY_P_H

