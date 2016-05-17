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

#ifndef TIMELOGCATEGORY_H
#define TIMELOGCATEGORY_H

#include <QUuid>

#include "TimeLogCategoryData.h"

struct TimeLogCategory : public TimeLogCategoryData
{
    typedef TimeLogCategoryData SUPER;
public:
    explicit TimeLogCategory(const QUuid &uuid = QUuid(),
                             const TimeLogCategoryData &data = TimeLogCategoryData());

    bool isValid() const;

    QString toString() const;

    QUuid uuid;
};

QDataStream &operator<<(QDataStream &stream, const TimeLogCategory &data);
QDataStream &operator>>(QDataStream &stream, TimeLogCategory &data);

QDebug &operator<<(QDebug &stream, const TimeLogCategory &data);

Q_DECLARE_TYPEINFO(TimeLogCategory, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TimeLogCategory)

#endif // TIMELOGCATEGORY_H
