#include <QLoggingCategory>

#include "TimeLogHistory.h"
#include "TimeLogHistoryWorker.h"

Q_LOGGING_CATEGORY(TIME_LOG_HISTORY_CATEGORY, "TimeLogHistory", QtInfoMsg)

TimeLogHistory::TimeLogHistory(QObject *parent) :
    QObject(parent),
    m_worker(new TimeLogHistoryWorker(this))
{
    connect(m_worker, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
    connect(m_worker, SIGNAL(dataAvailable(QVector<TimeLogEntry>)),
            this, SIGNAL(dataAvailable(QVector<TimeLogEntry>)));
}

TimeLogHistory::~TimeLogHistory()
{

}

bool TimeLogHistory::init()
{
    return m_worker->init();
}

qlonglong TimeLogHistory::size() const
{
    return m_worker->size();
}

QSet<QString> TimeLogHistory::categories() const
{
    return m_worker->categories();
}

void TimeLogHistory::insert(const TimeLogEntry &data)
{
    m_worker->insert(data);
}

void TimeLogHistory::remove(const QUuid &uuid)
{
    m_worker->remove(uuid);
}

void TimeLogHistory::edit(const TimeLogEntry &data)
{
    m_worker->edit(data);
}

void TimeLogHistory::getHistory(const QDateTime &begin, const QDateTime &end, const QString &category) const
{
    m_worker->getHistory(begin, end, category);
}

void TimeLogHistory::getHistory(const uint limit, const QDateTime &until) const
{
    m_worker->getHistory(limit, until);
}
