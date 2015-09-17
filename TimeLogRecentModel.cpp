#include "TimeLogRecentModel.h"

static const int defaultPopulateCount(5);

bool startTimeCompare(const TimeLogEntry &a, const TimeLogEntry &b)
{
    return a.startTime < b.startTime;
}

TimeLogRecentModel::TimeLogRecentModel(QObject *parent) :
    SUPER(parent)
{

}

bool TimeLogRecentModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent != QModelIndex()) {
        return false;
    }

    return m_history->size() > m_timeLog.size();
}

void TimeLogRecentModel::fetchMore(const QModelIndex &parent)
{
    if (parent != QModelIndex()) {
        return;
    }

    getMoreHistory();
}

void TimeLogRecentModel::processHistoryData(QVector<TimeLogEntry> data)
{
    if (!data.size()) {
        return;
    }

    int index = 0;

    if (!m_timeLog.isEmpty() && !startTimeCompare(data.last(), m_timeLog.first())) {
        QVector<TimeLogEntry>::iterator it = std::lower_bound(m_timeLog.begin(), m_timeLog.end(),
                                                              data.last(), startTimeCompare);
        index = it - m_timeLog.begin();
    }

    beginInsertRows(QModelIndex(), index, index + data.size() - 1);
    if (index == 0) {
        data.append(m_timeLog);
        m_timeLog.swap(data);
    } else {
        qCWarning(TIME_LOG_MODEL_CATEGORY) << "Inserting data not into beginning, current data:\n"
                                           << m_timeLog.first().startTime << "-" << m_timeLog.last().startTime
                                           << "\nnew data:\n"
                                           << data.first().startTime << "-" << data.last().startTime;
        m_timeLog.insert(index, data.size(), TimeLogEntry());
        for (int i = 0; i < data.size(); i++) {
            m_timeLog[index+i] = data.at(i);
        }
    }
    endInsertRows();
}

int TimeLogRecentModel::findData(const TimeLogEntry &entry) const
{
    QVector<TimeLogEntry>::const_iterator it = std::lower_bound(m_timeLog.begin(), m_timeLog.end(),
                                                                entry, startTimeCompare);
    if (it == m_timeLog.end() || it->uuid != entry.uuid) {
        qCWarning(TIME_LOG_MODEL_CATEGORY) << "Item absent or start time changed:\n"
                                           << entry.startTime << entry.category << entry.uuid;
        return -1;
    } else {
        return (it - m_timeLog.begin());
    }
}

void TimeLogRecentModel::getMoreHistory()
{
    QDateTime until = m_timeLog.size() ? m_timeLog.at(0).startTime : QDateTime::currentDateTime();
    if (m_requestedData.contains(until)) {
        qCDebug(TIME_LOG_MODEL_CATEGORY) << "Alredy requested data for time" << until;
        return;
    }
    m_requestedData.insert(until);
    m_history->getHistoryBefore(defaultPopulateCount, until);
}
