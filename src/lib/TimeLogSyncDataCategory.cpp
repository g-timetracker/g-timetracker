#include <QDebug>

#include "TimeLogSyncDataCategory.h"

TimeLogSyncDataCategory::TimeLogSyncDataCategory(const TimeLogCategory &data, const QDateTime &mTime) :
    sync(TimeLogSyncDataBase::Category, mTime),
    category(data)
{

}

TimeLogSyncDataCategory::TimeLogSyncDataCategory(const TimeLogSyncDataBase &base, const TimeLogCategory &category) :
    sync(base),
    category(category)
{

}

QString TimeLogSyncDataCategory::toString() const
{
    if (category.isValid()) {
        return QString("<CATEGORY> name: %1, data: %2, uuid: %3, mTime: %4 (%5)")
                .arg(category.name).arg(QVariant(category.data).toString()).arg(category.uuid.toString())
                .arg(sync.mTime.toMSecsSinceEpoch()).arg(sync.mTime.toString());
    } else if (!category.uuid.isNull()) {
        return QString("<DELETED CATEGORY> uuid: %1, mTime: %2 (%3)")
                .arg(category.uuid.toString()).arg(sync.mTime.toMSecsSinceEpoch()).arg(sync.mTime.toString());
    } else {
        return QString("<NONE>");
    }
}

QDataStream &operator<<(QDataStream &stream, const TimeLogSyncDataCategory &data)
{
    return stream << data.sync << data.category;
}

QDebug &operator<<(QDebug &stream, const TimeLogSyncDataCategory &data)
{
    return stream << data.toString();
}
