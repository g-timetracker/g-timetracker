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

#include "TimeLogRecentModel.h"

static const int defaultPopulateCount(5);

TimeLogRecentModel::TimeLogRecentModel(QObject *parent) :
    SUPER(parent),
    m_moreDataRequested(false),
    m_availableSize(0)
{

}

bool TimeLogRecentModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent != QModelIndex()) {
        return false;
    }

    bool result = m_history ? m_history->size() > m_timeLog.size() : false;
    if (!result) {
        m_moreDataRequested = true;
    }

    return result;
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
    if (m_timeLog.isEmpty() || startTimeCompare(data, m_timeLog.first())) {
        if (m_moreDataRequested) {
            m_moreDataRequested = false;
        } else {
            return;
        }
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

void TimeLogRecentModel::setHistory(TimeLogHistory *history)
{
    if (m_history == history) {
        return;
    }

    if (m_history) {
        disconnect(m_history, SIGNAL(sizeChanged(qlonglong)),
                   this, SLOT(setAvailableSize(qlonglong)));
    }

    setAvailableSize(0);

    SUPER::setHistory(history);

    if (history) {
        connect(history, SIGNAL(sizeChanged(qlonglong)),
                this, SLOT(setAvailableSize(qlonglong)));

        setAvailableSize(history->size());
    }
}

void TimeLogRecentModel::setAvailableSize(qlonglong availableSize)
{
    if (m_availableSize == availableSize) {
        return;
    }

    m_availableSize = availableSize;

    emit availableSizeChanged(m_availableSize);
}

void TimeLogRecentModel::getMoreHistory()
{
    if (!m_history) {
        return;
    }

    if (!m_pendingRequests.isEmpty()) {
        qCDebug(TIME_LOG_MODEL_CATEGORY) << "Data already requested";
        return;
    }

    QDateTime until = m_timeLog.size() ? m_timeLog.at(0).startTime : QDateTime::currentDateTimeUtc();
    qlonglong id = QDateTime::currentMSecsSinceEpoch();
    m_pendingRequests.append(id);
    m_history->getHistoryBefore(id, defaultPopulateCount, until);
}
