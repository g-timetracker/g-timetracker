#include "TimeLogSyncData.h"

TimeLogSyncData::TimeLogSyncData(const TimeLogEntry &entry, const QDateTime &mTime) :
    TimeLogEntry(entry),
    mTime(mTime)
{

}

QString TimeLogSyncData::toString() const
{
    if (isValid()) {
        return QString("<ITEM> start: %1, category: %2, comment: %3, mTime: %4 (%5), uuid: %6")
                .arg(startTime.toString()).arg(category).arg(comment)
                .arg(mTime.toMSecsSinceEpoch()).arg(mTime.toString()).arg(uuid.toString());
    } else if (!uuid.isNull()) {
        return QString("<DELETED> mTime: %1 (%2), uuid: %3")
                .arg(mTime.toMSecsSinceEpoch()).arg(mTime.toString()).arg(uuid.toString());
    } else {
        return QString("<NONE>");
    }
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
    return stream << data.toString();
}
