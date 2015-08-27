#ifndef TIMELOGDATA_H
#define TIMELOGDATA_H

#include <QObject>
#include <QDateTime>

struct TimeLogData
{
    Q_GADGET

    Q_PROPERTY(QDateTime startTime MEMBER startTime)
    Q_PROPERTY(int durationTime MEMBER durationTime)
    Q_PROPERTY(QString category MEMBER category)
    Q_PROPERTY(QString comment MEMBER comment)
public:
    TimeLogData();
    TimeLogData(QDateTime startTime, int durationTime, QString category, QString comment);

    QDateTime startTime;
    int durationTime;
    QString category;
    QString comment;
};

Q_DECLARE_TYPEINFO(TimeLogData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TimeLogData)

#endif // TIMELOGDATA_H
