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

QPointF TimeLog::mapToGlobal(QQuickItem *item)
{
    if (!item) {
        return QPointF();
    }

    return (item->window()->mapToGlobal(QPoint()) + item->mapToScene(QPointF()));
}
