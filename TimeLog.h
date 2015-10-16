#ifndef TIMELOG_H
#define TIMELOG_H

#include <QObject>
#include <QVariant>
#include <QPointF>

#include "TimeLogData.h"
#include "TimeLogStats.h"

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
    Q_INVOKABLE static void getStats(const QDateTime &begin = QDateTime::fromTime_t(0),
                                     const QDateTime &end = QDateTime::currentDateTime());
    Q_INVOKABLE static QPointF mapToGlobal(QQuickItem *item);

signals:
    void error(const QString &errorText) const;
    void statsDataAvailable(QVariantMap data, QDateTime until) const;

private slots:
    void statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const;
};

#endif // TIMELOG_H
