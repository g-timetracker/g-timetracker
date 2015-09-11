#ifndef TIMELOG_H
#define TIMELOG_H

#include <QObject>
#include <QPointF>

#include "TimeLogData.h"

class QQuickItem;

class TimeLog : public QObject
{
    Q_OBJECT
protected:
    explicit TimeLog(QObject *parent = 0);

public:
    virtual ~TimeLog();

    static TimeLog *instance();

    Q_INVOKABLE static TimeLogData createTimeLogData(QDateTime startTime, int durationTime,
                                                     QString category, QString comment);
    Q_INVOKABLE static QStringList categories();
    Q_INVOKABLE static QPointF mapToGlobal(QQuickItem *item);

signals:
    void error(const QString &errorText) const;
};

#endif // TIMELOG_H
