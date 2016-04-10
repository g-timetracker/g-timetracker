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
