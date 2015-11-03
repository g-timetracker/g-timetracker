#include "TimeLogSearchModel.h"

TimeLogSearchModel::TimeLogSearchModel(QObject *parent) :
    SUPER(parent),
    m_begin(QDateTime::fromTime_t(0)),
    m_end(QDateTime::fromTime_t(0))
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
    clear();
    m_requestedData.insert(m_end);
    m_history->getHistoryBetween(m_begin, m_end, m_category);
}

void TimeLogSearchModel::processDataInsert(QVector<TimeLogEntry> data)
{
    for (int i = 0; i < data.size(); i++) {
        const TimeLogEntry &entry = data.at(i);

        if (entry.startTime < m_begin || entry.startTime > m_end
            || (!m_category.isEmpty() && !entry.category.startsWith(m_category))) {
            continue;
        }

        QVector<TimeLogEntry>::iterator it = std::lower_bound(m_timeLog.begin(), m_timeLog.end(),
                                                              entry, startTimeCompare);
        if (it != m_timeLog.end() && it->uuid == entry.uuid) {
            continue;
        }
        int index = (it == m_timeLog.end() ? m_timeLog.size() : it - m_timeLog.begin());

        beginInsertRows(QModelIndex(), index, index);
        m_timeLog.insert(index, entry);
        endInsertRows();
    }
}
