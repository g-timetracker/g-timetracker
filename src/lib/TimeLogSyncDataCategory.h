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
