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
