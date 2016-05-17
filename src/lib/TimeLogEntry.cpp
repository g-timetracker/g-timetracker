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
