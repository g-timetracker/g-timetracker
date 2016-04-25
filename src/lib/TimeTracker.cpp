#include <QStandardPaths>
#include <QDir>

#include "TimeTracker.h"
#include "TimeLogHistory.h"
#include "TimeLogCategoryTreeNode.h"
#include "DataSyncer.h"

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
        id(Seconds), full("seconds"), abbreviated("s"), value(1) { }
    TimeUnit(TimeUnits id, const QString &full, const QString &abbreviated, int value) :
        id(id), full(full), abbreviated(abbreviated), value(value) { }

    TimeUnits id;
    QString full;
    QString abbreviated;
    int value;
};

static const QVector<TimeUnit> timeUnits = QVector<TimeUnit>()
        << TimeUnit(Seconds, "seconds", "s", 1)
        << TimeUnit(Minutes, "minutes", "min", 60)
        << TimeUnit(Hours, "hours", "hr", 60 * 60)
        << TimeUnit(Days, "days", "d", 24 * 60 * 60)
        << TimeUnit(Weeks, "weeks", "wk", 7 * 24 * 60 * 60)
        << TimeUnit(Months, "months", "mo", 30 * 24 * 60 * 60)
        << TimeUnit(Years, "years", "yr", 365 * 24 * 60 * 60);

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

TimeTracker::TimeTracker(QObject *parent) :
    QObject(parent),
    m_history(Q_NULLPTR),
    m_syncer(Q_NULLPTR),
    m_undoCount(0)
{

}

void TimeTracker::setDataPath(const QUrl &dataPathUrl)
{
    if (m_dataPath.path() == dataPathUrl.path() && m_history && m_syncer) {
        return;
    }

    m_dataPath = dataPathUrl;

    TimeLogHistory *history = new TimeLogHistory(this);
    if (!history->init(dataPathUrl.path(), QString(), true)) {
        emit error(tr("Fail to initialize DB"));
        delete history;
        return;
    }
    setHistory(history);

    DataSyncer *syncer = new DataSyncer(m_history, this);
    syncer->init(dataPathUrl.path());
    setSyncer(syncer);

    emit dataPathChanged(m_dataPath);
}

TimeLogHistory *TimeTracker::history()
{
    return m_history;
}

QSharedPointer<TimeLogCategoryTreeNode> TimeTracker::categories() const
{
    return m_categories;
}

int TimeTracker::undoCount() const
{
    return m_undoCount;
}

TimeLogData TimeTracker::createTimeLogData(QDateTime startTime, QString category, QString comment)
{
    // Clear milliseconds to make it equal to fromTime_t()
    QTime time(startTime.time());
    time.setHMS(time.hour(), time.minute(), time.second(), 0);
    startTime.setTime(time);

    return TimeLogData(startTime.toUTC(), category, comment);
}

TimeLogCategoryData TimeTracker::createTimeLogCategoryData(QString name, QVariantMap data)
{
    return TimeLogCategoryData(name, data);
}

void TimeTracker::undo()
{
    if (!m_history) {
        return;
    }

    m_history->undo();
}

void TimeTracker::getStats(const QDateTime &begin, const QDateTime &end, const QString &category, const QString &separator)
{
    if (!m_history) {
        return;
    }

    if (!begin.isValid() || !end.isValid()) {
        return;
    }

    m_history->getStats(begin, end, category, separator);
}

QString TimeTracker::durationText(int duration, int maxUnits, bool isAbbreviate)
{
    QStringList values;
    while (--maxUnits >= 0) {
        int unit = calcTimeUnits(duration);
        QString value;
        value.setNum(duration / timeUnits.at(unit).value);
        values.append(QString("%1 %2").arg(value).arg(isAbbreviate ? timeUnits.at(unit).abbreviated
                                                                   : timeUnits.at(unit).full));
        duration %= timeUnits.at(unit).value;
        if (!duration) {
            break;
        }
    }

    return values.join(", ");
}

QUrl TimeTracker::documentsLocation()
{
    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
}

bool TimeTracker::createFolder(const QString &path, const QString &name)
{
    return QDir(path).mkdir(name);
}

void TimeTracker::addCategory(const TimeLogCategory &category)
{
    if (!m_history) {
        return;
    }

    m_history->addCategory(category);
}

void TimeTracker::removeCategory(const QString &name)
{
    if (!m_history) {
        return;
    }

    m_history->removeCategory(name);
}

void TimeTracker::editCategory(const QString &oldName, const TimeLogCategory &category)
{
    if (!m_history) {
        return;
    }

    m_history->editCategory(oldName, category);
}

void TimeTracker::statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const
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
    result.insert("units", timeUnits.at(unit).abbreviated);
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

void TimeTracker::updateCategories(const QSharedPointer<TimeLogCategoryTreeNode> &categories)
{
    m_categories = categories;

    emit categoriesChanged(m_categories);
}

void TimeTracker::updateUndoCount(int undoCount)
{
    if (m_undoCount == undoCount) {
        return;
    }

    m_undoCount = undoCount;

    emit undoCountChanged(m_undoCount);
}

void TimeTracker::setHistory(TimeLogHistory *history)
{
    if (m_history == history) {
        return;
    }

    TimeLogHistory *oldHistory = m_history;

    m_history = history;

    if (history) {
        connect(m_history, SIGNAL(error(QString)),
                this, SIGNAL(error(QString)));
        connect(m_history, SIGNAL(statsDataAvailable(QVector<TimeLogStats>,QDateTime)),
                this, SLOT(statsDataAvailable(QVector<TimeLogStats>,QDateTime)));
        connect(m_history, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)),
                this, SLOT(updateCategories(QSharedPointer<TimeLogCategoryTreeNode>)));
        connect(m_history, SIGNAL(undoCountChanged(int)),
                this, SLOT(updateUndoCount(int)));
    }

    emit historyChanged(m_history);

    updateCategories(m_history ? m_history->categories() : QSharedPointer<TimeLogCategoryTreeNode>());
    updateUndoCount(m_history ? m_history->undoCount() : 0);

    if (oldHistory) {
        delete oldHistory;
    }
}

void TimeTracker::setSyncer(DataSyncer *syncer)
{
    if (m_syncer == syncer) {
        return;
    }

    DataSyncer *oldSyncer = m_syncer;

    m_syncer = syncer;

    if (syncer) {
        connect(m_syncer, SIGNAL(error(QString)),
                this, SIGNAL(error(QString)));
    }

    emit syncerChanged(m_syncer);

    if (oldSyncer) {
        delete oldSyncer;
    }
}
