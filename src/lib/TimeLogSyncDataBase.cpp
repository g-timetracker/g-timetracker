#include <QDataStream>

#include "TimeLogSyncDataBase.h"

TimeLogSyncDataBase::TimeLogSyncDataBase(Type type, QDateTime mTime, bool isRemoved) :
    mTime(mTime),
    type(type),
    isRemoved(isRemoved)
{

}

QDataStream &operator<<(QDataStream &stream, const TimeLogSyncDataBase &data)
{
    return stream << static_cast<qint32>(data.type) << data.mTime;
}

QDataStream &operator>>(QDataStream &stream, TimeLogSyncDataBase &data)
{
    qint32 type = static_cast<qint32>(TimeLogSyncDataBase::Invalid);
    stream >> type >> data.mTime;
    data.type = static_cast<TimeLogSyncDataBase::Type>(type);

    return stream;
}
