#include <QDebug>

#include "TimeLogSyncDataEntry.h"

TimeLogSyncDataEntry::TimeLogSyncDataEntry(const TimeLogEntry &entry, const QDateTime &mTime) :
    sync(TimeLogSyncDataBase::Entry, mTime),
    entry(entry)
{

}

TimeLogSyncDataEntry::TimeLogSyncDataEntry(const TimeLogSyncDataBase &base, const TimeLogEntry &entry) :
    sync(base),
    entry(entry)
{

}

QString TimeLogSyncDataEntry::toString() const
{
    if (entry.isValid()) {
        return QString("<ENTRY> start: %1, category: %2, comment: %3, uuid: %4, mTime: %5 (%6)")
                .arg(entry.startTime.toString()).arg(entry.category).arg(entry.comment)
                .arg(entry.uuid.toString()).arg(sync.mTime.toMSecsSinceEpoch()).arg(sync.mTime.toString());
    } else if (!entry.uuid.isNull()) {
        return QString("<DELETED ENTRY> uuid: %1, mTime: %2 (%3)")
                .arg(entry.uuid.toString()).arg(sync.mTime.toMSecsSinceEpoch()).arg(sync.mTime.toString());
    } else {
        return QString("<NONE>");
    }
}

QDataStream &operator<<(QDataStream &stream, const TimeLogSyncDataEntry &data)
{
    return stream << data.sync << data.entry;
}

QDebug &operator<<(QDebug &stream, const TimeLogSyncDataEntry &data)
{
    return stream << data.toString();
}
