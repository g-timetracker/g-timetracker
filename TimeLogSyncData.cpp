#include "TimeLogSyncData.h"

TimeLogSyncData::TimeLogSyncData(const TimeLogEntry &entry, const QDateTime &mTime) :
    TimeLogEntry(entry),
    mTime(mTime)
{

}

QDataStream &operator<<(QDataStream &stream, const TimeLogSyncData &data)
{
    return stream << data.mTime << data.uuid << data.startTime << data.category << data.comment;
}

QDataStream &operator>>(QDataStream &stream, TimeLogSyncData &data)
{
    return stream >> data.mTime >> data.uuid >> data.startTime >> data.category >> data.comment;
}

QDebug &operator<<(QDebug &stream, const TimeLogSyncData &data)
{
    return stream << data.uuid << data.startTime << data.category << data.comment
                  << data.mTime.toMSecsSinceEpoch();
}
