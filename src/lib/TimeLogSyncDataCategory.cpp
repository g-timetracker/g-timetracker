/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

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
