#include "TimeLogRecentModel.h"

static const int defaultPopulateCount(5);

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

void TimeLogRecentModel::processDataInsert(TimeLogEntry data)
{
    if (m_timeLog.size() > defaultPopulateCount && startTimeCompare(data, m_timeLog.first())) {
        return;
    }

    QVector<TimeLogEntry>::iterator it = std::lower_bound(m_timeLog.begin(), m_timeLog.end(),
                                                          data, startTimeCompare);
    if (it != m_timeLog.end() && it->uuid == data.uuid) {
        return;
    }
    int index = (it == m_timeLog.end() ? m_timeLog.size() : it - m_timeLog.begin());

    beginInsertRows(QModelIndex(), index, index);
    m_timeLog.insert(index, data);
    endInsertRows();
}

int TimeLogRecentModel::findData(const TimeLogEntry &entry) const
{
    QVector<TimeLogEntry>::const_iterator it = std::lower_bound(m_timeLog.begin(), m_timeLog.end(),
                                                                entry, startTimeCompare);
    if (it == m_timeLog.end() || it->uuid != entry.uuid) {
        return -1;
    } else {
        return (it - m_timeLog.begin());
    }
}

void TimeLogRecentModel::getMoreHistory()
{
    if (!m_pendingRequests.isEmpty()) {
        qCDebug(TIME_LOG_MODEL_CATEGORY) << "Data already requested";
        return;
    }

    QDateTime until = m_timeLog.size() ? m_timeLog.at(0).startTime : QDateTime::currentDateTime();
    qlonglong id = QDateTime::currentMSecsSinceEpoch();
    m_pendingRequests.append(id);
    m_history->getHistoryBefore(id, defaultPopulateCount, until);
}
