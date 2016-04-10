#ifndef TIMELOGENTRY_H
#define TIMELOGENTRY_H

#include <QUuid>

#include "TimeLogData.h"

struct TimeLogEntry: public TimeLogData
{
    typedef TimeLogData SUPER;
public:
    explicit TimeLogEntry(const QUuid &uuid = QUuid(), const TimeLogData &data = TimeLogData());

    bool isValid() const;

    QString toString() const;

    QUuid uuid;
    int durationTime;
    QDateTime precedingStart;
};

QDataStream &operator<<(QDataStream &stream, const TimeLogEntry &data);
QDataStream &operator>>(QDataStream &stream, TimeLogEntry &data);

QDebug &operator<<(QDebug &stream, const TimeLogEntry &data);

Q_DECLARE_TYPEINFO(TimeLogEntry, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TimeLogEntry)

#endif // TIMELOGENTRY_H
