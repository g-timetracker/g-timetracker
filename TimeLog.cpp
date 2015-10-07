#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>

#include "TimeLog.h"
#include "TimeLog_p.h"
#include "TimeLogHistory.h"
#include "DataSyncer.h"

Q_GLOBAL_STATIC(TimeLogSingleton, timeLog)

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
    QVariantList result;

    foreach (const TimeLogStats &entry, data) {
        QVariantMap map;
        map.insert("label", entry.category);
        QVariantList datasetData;
        datasetData.append(entry.durationTime);
        map.insert("data", datasetData);
        result.append(map);
    }

    emit statsDataAvailable(result, until);
}
