#ifndef TIMELOGDATA_H
#define TIMELOGDATA_H

#include <QObject>
#include <QDateTime>

struct TimeLogData
{
    Q_GADGET

    Q_PROPERTY(QString category MEMBER category)
public:
    TimeLogData();
    TimeLogData(QDateTime startTime, int durationTime, QString category, QString comment);

    QDateTime startTime;
    int durationTime;
    QString category;
    QString comment;
};

Q_DECLARE_METATYPE(TimeLogData)

#endif // TIMELOGDATA_H
