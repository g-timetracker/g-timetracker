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
