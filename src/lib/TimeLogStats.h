#ifndef TIMELOGSTATS_H
#define TIMELOGSTATS_H

#include <QObject>

struct TimeLogStats
{
    Q_GADGET

    Q_PROPERTY(int durationTime MEMBER durationTime)
    Q_PROPERTY(QString category MEMBER category)

public:
    TimeLogStats();
    TimeLogStats(int durationTime, const QString &category);

    int durationTime;
    QString category;
};

Q_DECLARE_TYPEINFO(TimeLogStats, Q_MOVABLE_TYPE);

#endif // TIMELOGSTATS_H
