#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>

#include "TimeLog.h"
#include "TimeLog_p.h"
#include "TimeLogHistory.h"
#include "DataSyncer.h"

Q_GLOBAL_STATIC(TimeLogSingleton, timeLog)

bool durationTimeCompare(const TimeLogStats &a, const TimeLogStats &b)
{
    return a.durationTime < b.durationTime;
}

QPair<QString, int> calcTimeUnits(int duration)
{
    if (duration / (365 * 24 * 60 * 60)) {
        return QPair<QString, int>("years", 365 * 24 * 60 * 60);
    } else if (duration / (30 * 24 * 60 * 60)) {
        return QPair<QString, int>("months", 30 * 24 * 60 * 60);
    } else if (duration / (7 * 24 * 60 * 60)) {
        return QPair<QString, int>("weeks", 7 * 24 * 60 * 60);
    } else if (duration / (24 * 60 * 60)) {
        return QPair<QString, int>("days", 24 * 60 * 60);
    } else if (duration / (60 * 60)) {
        return QPair<QString, int>("hours", 60 * 60);
    } else if (duration / (60)) {
        return QPair<QString, int>("minutes", 60);
    } else {
        return QPair<QString, int>("seconds", 1);
    }
}

TimeLog::TimeLog(QObject *parent) : QObject(parent)
{
    connect(TimeLogHistory::instance(), SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
    connect(TimeLogHistory::instance(), SIGNAL(statsDataAvailable(QVector<TimeLogStats>,QDateTime)),
            this, SLOT(statsDataAvailable(QVector<TimeLogStats>,QDateTime)));
    connect(DataSyncer::instance(), SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
}

TimeLog::~TimeLog()
{

}

TimeLog *TimeLog::instance()
{
    return static_cast<TimeLog*>(timeLog);
}

TimeLogData TimeLog::createTimeLogData(QDateTime startTime, int durationTime,
                                       QString category, QString comment)
{
    return TimeLogData(startTime, durationTime, category, comment);
}

QStringList TimeLog::categories()
{
    QStringList result = TimeLogHistory::instance()->categories().toList();
    std::sort(result.begin(), result.end());

    return result;
}

void TimeLog::getStats(const QDateTime &begin, const QDateTime &end)
{
    TimeLogHistory::instance()->getStats(begin, end);
}

QPointF TimeLog::mapToGlobal(QQuickItem *item)
{
    if (!item) {
        return QPointF();
    }

    return (item->window()->mapToGlobal(QPoint()) + item->mapToScene(QPointF()));
}

void TimeLog::statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const
{
    QVariantMap result;

    QPair<QString, int> timeUnits("", 1);
    int maxValue = 0;
    if (!data.isEmpty()) {
        QVector<TimeLogStats>::const_iterator max = std::max_element(data.cbegin(), data.cend(),
                                                                     durationTimeCompare);
        maxValue = max->durationTime;
    }
    timeUnits = calcTimeUnits(maxValue);
    result.insert("units", timeUnits.first);
    result.insert("max", static_cast<float>(maxValue) / timeUnits.second);

    QVariantList dataset;
    foreach (const TimeLogStats &entry, data) {
        QVariantMap map;
        map.insert("label", entry.category);
        map.insert("value", static_cast<float>(entry.durationTime) / timeUnits.second);
        dataset.append(map);
    }
    result.insert("data", dataset);

    emit statsDataAvailable(result, until);
}
