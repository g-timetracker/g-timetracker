#include "TimeLogSearchModel.h"

TimeLogSearchModel::TimeLogSearchModel(QObject *parent) :
    SUPER(parent),
    m_begin(QDateTime::currentDateTime()),
    m_end(QDateTime::currentDateTime())
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
    qlonglong id = QDateTime::currentMSecsSinceEpoch();
    m_pendingRequests.append(id);
    m_history->getHistoryBetween(id, m_begin, m_end, m_category);
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
