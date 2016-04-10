#include <QDebug>

#include "TimeLogEntry.h"

TimeLogEntry::TimeLogEntry(const QUuid &uuid, const TimeLogData &data) :
    TimeLogData(data),
    uuid(uuid),
    durationTime(0),
    precedingStart()
{

}

bool TimeLogEntry::isValid() const
{
    return (SUPER::isValid() && !uuid.isNull());
}

QString TimeLogEntry::toString() const
{
    return QString("start: %1, category: %2, comment: %3, precedingStart: %4, duration: %5, uuid: %6")
            .arg(startTime.toString()).arg(category).arg(comment)
            .arg(precedingStart.toString()).arg(durationTime).arg(uuid.toString());
}

QDebug &operator<<(QDebug &stream, const TimeLogEntry &data)
{
    return stream << data.toString();
}

QDataStream &operator<<(QDataStream &stream, const TimeLogEntry &data)
{
    return stream << data.uuid << data.startTime << data.category << data.comment;
}

QDataStream &operator>>(QDataStream &stream, TimeLogEntry &data)
{
    return stream >> data.uuid >> data.startTime >> data.category >> data.comment;
}
