#ifndef TIMELOGSINGLETON_H
#define TIMELOGSINGLETON_H

#include <QObject>

#include "TimeLogData.h"

class TimeLogSingleton : public QObject
{
    Q_OBJECT
public:
    explicit TimeLogSingleton(QObject *parent = 0);

    Q_INVOKABLE static TimeLogData createTimeLogData(QDateTime startTime, int durationTime,
                                                     QString category, QString comment);

signals:
    void error(const QString &errorText) const;
};

#endif // TIMELOGSINGLETON_H
