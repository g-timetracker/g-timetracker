#ifndef TIMELOGSYNCDATAENTRY_H
#define TIMELOGSYNCDATAENTRY_H

#include "TimeLogSyncDataBase.h"
#include "TimeLogEntry.h"

struct TimeLogSyncDataEntry
{
public:
    TimeLogSyncDataEntry(const TimeLogEntry &entry = TimeLogEntry(), const QDateTime &mTime = QDateTime());
    TimeLogSyncDataEntry(const TimeLogSyncDataBase &base, const TimeLogEntry &entry);

    QString toString() const;

    TimeLogSyncDataBase sync;
    TimeLogEntry entry;
};

QDataStream &operator<<(QDataStream &stream, const TimeLogSyncDataEntry &data);

QDebug &operator<<(QDebug &stream, const TimeLogSyncDataEntry &data);

Q_DECLARE_TYPEINFO(TimeLogSyncDataEntry, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TimeLogSyncDataEntry)

#endif // TIMELOGSYNCDATAENTRY_H
