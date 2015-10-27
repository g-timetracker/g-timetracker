#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>

#include "TimeLog.h"
#include "TimeLog_p.h"
#include "TimeLogHistory.h"
#include "DataSyncer.h"

Q_GLOBAL_STATIC(TimeLogSingleton, timeLog)

enum TimeUnits {
    Seconds = 0,
    Minutes,
    Hours,
    Days,
    Weeks,
    Months,
    Years
};

struct TimeUnit {
    TimeUnit() :
        id(Seconds), name("seconds"), value(1) { }
    TimeUnit(TimeUnits id, QString name, int value) :
        id(id), name(name), value(value) { }

    TimeUnits id;
    QString name;
    int value;
};

static const QVector<TimeUnit> timeUnits = QVector<TimeUnit>() << TimeUnit(Seconds, "seconds", 1)
                                                               << TimeUnit(Minutes, "minutes", 60)
                                                               << TimeUnit(Hours, "hours", 60 * 60)
                                                               << TimeUnit(Days, "days", 24 * 60 * 60)
                                                               << TimeUnit(Weeks, "weeks", 7 * 24 * 60 * 60)
                                                               << TimeUnit(Months, "months", 30 * 24 * 60 * 60)
                                                               << TimeUnit(Years, "years", 365 * 24 * 60 * 60);

bool durationTimeCompare(const TimeLogStats &a, const TimeLogStats &b)
{
    return a.durationTime < b.durationTime;
}

int calcTimeUnits(int duration)
{
    int i;
    for (i = timeUnits.size() - 1; i > 0; i--) {
        if (duration / timeUnits.at(i).value) {
            break;
        }
    }

    return i;
}

TimeLog::TimeLog(QObject *parent) : QObject(parent)
{
    connect(TimeLogHistory::instance(), SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
    connect(TimeLogHistory::instance(), SIGNAL(statsDataAvailable(QVector<TimeLogStats>,QDateTime)),
            this, SLOT(statsDataAvailable(QVector<TimeLogStats>,QDateTime)));
    connect(TimeLogHistory::instance(), SIGNAL(categoriesChanged(QSet<QString>)),
            this, SLOT(categoriesAvailable(QSet<QString>)));
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

QStringList TimeLog::categories()
{
    QStringList result = TimeLogHistory::instance()->categories().toList();
    std::sort(result.begin(), result.end());

    return result;
}

TimeLogData TimeLog::createTimeLogData(QDateTime startTime, int durationTime,
                                       QString category, QString comment)
{
    return TimeLogData(startTime, durationTime, category, comment);
}

void TimeLog::editCategory(QString oldName, QString newName)
{
    TimeLogHistory::instance()->editCategory(oldName, newName);
}

void TimeLog::getStats(const QDateTime &begin, const QDateTime &end, const QString &category, const QString &separator)
{
    TimeLogHistory::instance()->getStats(begin, end, category, separator);
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

    int unit = 0;
    int maxValue = 0;
    if (!data.isEmpty()) {
        QVector<TimeLogStats>::const_iterator max = std::max_element(data.cbegin(), data.cend(),
                                                                     durationTimeCompare);
        maxValue = max->durationTime;
    }
    unit = calcTimeUnits(maxValue);
    result.insert("units", timeUnits.at(unit).name);
    result.insert("max", static_cast<float>(maxValue) / timeUnits.at(unit).value);

    QVariantList dataset;
    foreach (const TimeLogStats &entry, data) {
        QVariantMap map;
        map.insert("label", entry.category);
        map.insert("value", static_cast<float>(entry.durationTime) / timeUnits.at(unit).value);
        dataset.append(map);
    }
    result.insert("data", dataset);

    emit statsDataAvailable(result, until);
}

void TimeLog::categoriesAvailable(QSet<QString> categories) const
{
    QStringList result = categories.toList();
    std::sort(result.begin(), result.end());

    emit categoriesChanged(result);
}
