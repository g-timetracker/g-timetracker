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

#ifndef TIMELOGSYNCDATABASE_H
#define TIMELOGSYNCDATABASE_H

#include <QDateTime>

struct TimeLogSyncDataBase
{
public:
    enum Type {
        Invalid     = 0,
        Entry       = 1,
        Category    = 2
    };

    TimeLogSyncDataBase(TimeLogSyncDataBase::Type type = TimeLogSyncDataBase::Invalid,
                        QDateTime mTime = QDateTime(), bool isRemoved = false);

    QDateTime mTime;
    Type type;
    bool isRemoved;
};

QDataStream &operator<<(QDataStream &stream, const TimeLogSyncDataBase &data);
QDataStream &operator>>(QDataStream &stream, TimeLogSyncDataBase &data);

Q_DECLARE_TYPEINFO(TimeLogSyncDataBase, Q_MOVABLE_TYPE);

#endif // TIMELOGSYNCDATABASE_H
