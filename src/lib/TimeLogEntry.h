#ifndef TIMELOGENTRY_H
#define TIMELOGENTRY_H

#include <QUuid>

#include <QDebug>

#include "TimeLogData.h"

struct TimeLogEntry: public TimeLogData
{
    typedef TimeLogData SUPER;
public:
    TimeLogEntry(const QUuid &uuid = QUuid(), const TimeLogData &data = TimeLogData());

    bool isValid() const;

    QString toString() const;

    QUuid uuid;
    int durationTime;
    QDateTime precedingStart;
};

QDebug &operator<<(QDebug &stream, const TimeLogEntry &data);

Q_DECLARE_TYPEINFO(TimeLogEntry, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TimeLogEntry)

#endif // TIMELOGENTRY_H
