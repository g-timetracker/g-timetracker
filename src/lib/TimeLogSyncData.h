#ifndef TIMELOGSYNCDATA_H
#define TIMELOGSYNCDATA_H

#include <QDebug>

#include "TimeLogEntry.h"

struct TimeLogSyncData : public TimeLogEntry
{
public:
    TimeLogSyncData(const TimeLogEntry &entry = TimeLogEntry(), const QDateTime &mTime = QDateTime());

    QString toString() const;

    QDateTime mTime;
};

QDataStream &operator<<(QDataStream &stream, const TimeLogSyncData &data);
QDataStream &operator>>(QDataStream &stream, TimeLogSyncData &data);

QDebug &operator<<(QDebug &stream, const TimeLogSyncData &data);

Q_DECLARE_TYPEINFO(TimeLogSyncData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TimeLogSyncData)

#endif // TIMELOGSYNCDATA_H
