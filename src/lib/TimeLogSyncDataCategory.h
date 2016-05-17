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

#ifndef TIMELOGSYNCDATACATEGORY_H
#define TIMELOGSYNCDATACATEGORY_H

#include <QDateTime>

#include "TimeLogSyncDataBase.h"
#include "TimeLogCategory.h"

struct TimeLogSyncDataCategory
{
public:
    TimeLogSyncDataCategory(const TimeLogCategory &data = TimeLogCategory(),
                            const QDateTime &mTime = QDateTime());
    TimeLogSyncDataCategory(const TimeLogSyncDataBase &base, const TimeLogCategory &category);

    QString toString() const;

    TimeLogSyncDataBase sync;
    TimeLogCategory category;
};

QDataStream &operator<<(QDataStream &stream, const TimeLogSyncDataCategory &data);

QDebug &operator<<(QDebug &stream, const TimeLogSyncDataCategory &data);

Q_DECLARE_TYPEINFO(TimeLogSyncDataCategory, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TimeLogSyncDataCategory)

#endif // TIMELOGSYNCDATACATEGORY_H
