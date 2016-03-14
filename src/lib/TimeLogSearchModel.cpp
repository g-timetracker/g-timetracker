#include "TimeLogSearchModel.h"

TimeLogSearchModel::TimeLogSearchModel(QObject *parent) :
    SUPER(parent),
    m_begin(QDateTime::currentDateTimeUtc()),
    m_end(QDateTime::currentDateTimeUtc())
{
    connect(this, SIGNAL(beginChanged(QDateTime)),
            this, SLOT(updateData()));
    connect(this, SIGNAL(endChanged(QDateTime)),
            this, SLOT(updateData()));
    connect(this, SIGNAL(categoryChanged(QString)),
            this, SLOT(updateData()));
}

void TimeLogSearchModel::updateData()
{
    if (!m_history) {
        return;
    }

    if (!m_begin.isValid() || !m_end.isValid()) {
        return;
    }

    clear();
    qlonglong id = QDateTime::currentMSecsSinceEpoch();
    m_pendingRequests.append(id);
    m_history->getHistoryBetween(id, m_begin, m_end, m_category);
}

void TimeLogSearchModel::processDataInsert(TimeLogEntry data)
{
    if (data.startTime < m_begin || data.startTime > m_end
        || (!m_category.isEmpty() && !data.category.startsWith(m_category))) {
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

int TimeLogSearchModel::findData(const TimeLogEntry &entry) const
{
    if (entry.startTime < m_begin || entry.startTime > m_end
        || (!m_category.isEmpty() && !entry.category.startsWith(m_category))) {
        return -1;
    }

    QVector<TimeLogEntry>::const_iterator it = std::lower_bound(m_timeLog.begin(), m_timeLog.end(),
                                                                entry, startTimeCompare);
    if (it == m_timeLog.end() || it->uuid != entry.uuid) {
        return -1;
    } else {
        return (it - m_timeLog.begin());
    }
}
