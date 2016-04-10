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
