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
