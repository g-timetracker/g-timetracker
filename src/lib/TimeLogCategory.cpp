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

#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>

#include "TimeLogCategory.h"

TimeLogCategory::TimeLogCategory(const QUuid &uuid, const TimeLogCategoryData &data) :
    TimeLogCategoryData(data),
    uuid(uuid)
{

}

bool TimeLogCategory::isValid() const
{
    return (SUPER::isValid() && !uuid.isNull());
}

QString TimeLogCategory::toString() const
{
    return QString("name: %1, data: %2, uuid: %3").arg(name)
            .arg(QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(data)).toJson(QJsonDocument::Compact)))
            .arg(uuid.toString());
}


QDataStream &operator<<(QDataStream &stream, const TimeLogCategory &data)
{
    return stream << data.uuid << data.name
                  << QJsonDocument(QJsonObject::fromVariantMap(data.data)).toBinaryData();
}

QDataStream &operator>>(QDataStream &stream, TimeLogCategory &data)
{
    QByteArray jsonData;

    stream >> data.uuid >> data.name >> jsonData;

    if (!jsonData.isNull()) {
        data.data = QJsonDocument::fromBinaryData(jsonData).object().toVariantMap();
    }

    return stream;
}

QDebug &operator<<(QDebug &stream, const TimeLogCategory &data)
{
    return stream << data.toString();
}
