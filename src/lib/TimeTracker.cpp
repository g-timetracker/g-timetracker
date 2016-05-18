/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QVector>
#include <QLocale>

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

// HACK: handle plurals
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(ctx, str, cmnt, n) str

struct TimeUnit {
    TimeUnit() :
        id(Seconds), full(QT_TRANSLATE_NOOP("duration", "%n second(s)", 0, 0)),
        abbreviated(QT_TRANSLATE_NOOP("duration", "%n s", 0, 0)), value(1) { }
    TimeUnit(TimeUnits id, const char *full, const char *abbreviated, int value) :
        id(id), full(full), abbreviated(abbreviated), value(value) { }

    TimeUnits id;
    const char *full;
    const char *abbreviated;
    int value;
};

static const QVector<TimeUnit> timeUnits = QVector<TimeUnit>()
        << TimeUnit(Seconds, QT_TRANSLATE_NOOP("duration", "%n second(s)", 0, 0),
                    QT_TRANSLATE_NOOP("duration", "%n s", 0, 0), 1)
        << TimeUnit(Minutes, QT_TRANSLATE_NOOP("duration", "%n minute(s)", 0, 0),
                    QT_TRANSLATE_NOOP("duration", "%n min", 0, 0), 60)
        << TimeUnit(Hours, QT_TRANSLATE_NOOP("duration", "%n hour(s)", 0, 0),
                    QT_TRANSLATE_NOOP("duration", "%n hr", 0, 0), 60 * 60)
        << TimeUnit(Days, QT_TRANSLATE_NOOP("duration", "%n day(s)", 0, 0),
                    QT_TRANSLATE_NOOP("duration", "%n d", 0, 0), 24 * 60 * 60)
        << TimeUnit(Weeks, QT_TRANSLATE_NOOP("duration", "%n week(s)", 0, 0),
                    QT_TRANSLATE_NOOP("duration", "%n wk", 0, 0), 7 * 24 * 60 * 60)
        << TimeUnit(Months, QT_TRANSLATE_NOOP("duration", "%n month(s)", 0, 0),
                    QT_TRANSLATE_NOOP("duration", "%n mo", 0, 0), 30 * 24 * 60 * 60)
        << TimeUnit(Years, QT_TRANSLATE_NOOP("duration", "%n year(s)", 0, 0),
                    QT_TRANSLATE_NOOP("duration", "%n yr", 0, 0), 365 * 24 * 60 * 60);

// restore hacked macro
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(ctx, str) str

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
    if (!history->init(dataPathUrl.path(), QString(), false, true)) {
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
        values.append(QCoreApplication::translate("duration",
                                                  isAbbreviate ? timeUnits.at(unit).abbreviated
                                                               : timeUnits.at(unit).full,
                                                  nullptr, duration / timeUnits.at(unit).value));
        duration %= timeUnits.at(unit).value;
        if (!duration) {
            break;
        }
    }

    return values.join(", ");
}

QString TimeTracker::rangeText(const QDateTime &from, const QDateTime &to)
{
    QDate fromDate(from.date());
    QDate toDate(to.date());

    if (fromDate == toDate) {
        return QString(fromDate.toString(Qt::DefaultLocaleShortDate) + " "
                       + from.time().toString(Qt::DefaultLocaleShortDate) + "\u2013"
                       + to.time().toString(Qt::DefaultLocaleShortDate));
    } else {
        return QString(from.toString(Qt::DefaultLocaleShortDate) + " \u2013 "
                       + to.toString(Qt::DefaultLocaleShortDate));
    }
}

QVariantList TimeTracker::weeksModel()
{
    QVariantList result;

    QDate start(QDate::currentDate());
    start = start.addDays(-(start.dayOfWeek() + 7 - QLocale().firstDayOfWeek()) % 7);
    QDate end = start.addYears(-1);

    QDate d(start.addDays(7));
    do {
        QVariantMap weekData;
        weekData.insert("to", d);
        d = d.addDays(-7);
        weekData.insert("from", d);
        weekData.insert("number", d.weekNumber());
        result.append(weekData);
    } while (d >= end);

    return result;
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
    result.insert("unitsName", timeUnits.at(unit).abbreviated);
    result.insert("unitsValue", timeUnits.at(unit).value);
    result.insert("max", maxValue);

    QVariantList dataset;
    foreach (const TimeLogStats &entry, data) {
        QVariantMap map;
        map.insert("label", entry.category);
        map.insert("value", entry.durationTime);
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
