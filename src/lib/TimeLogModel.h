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

#ifndef TIMELOGMODEL_H
#define TIMELOGMODEL_H

#include <QAbstractListModel>

#include <QLoggingCategory>

#include "TimeLogHistory.h"

class TimeTracker;

class TimeLogModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(TimeTracker* timeTracker MEMBER m_timeTracker WRITE setTimeTracker NOTIFY timeTrackerChanged)
    typedef QAbstractListModel SUPER;
public:
    enum Roles {
        StartTimeRole = Qt::UserRole + 1,
        DurationTimeRole,
        CategoryRole,
        CommentRole,
        PrecedingStartRole,
        SucceedingStartRole
    };

    explicit TimeLogModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    virtual bool removeRows(int row, int count, const QModelIndex &parent);

    Q_INVOKABLE void removeItem(const QModelIndex &index);
    Q_INVOKABLE void appendItem(TimeLogData data = TimeLogData());
    Q_INVOKABLE void insertItem(const QModelIndex &index, TimeLogData data = TimeLogData());

    void setTimeTracker(TimeTracker *timeTracker);

protected slots:
    virtual void setHistory(TimeLogHistory *history);

private slots:
    void historyDataOutdated();
    void historyRequestCompleted(QVector<TimeLogEntry> data, qlonglong id);
    void historyDataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields);
    void historyDataInserted(TimeLogEntry data);
    void historyDataRemoved(TimeLogEntry data);

signals:
    void timeTrackerChanged(TimeTracker *newTimeTracker);
    void error(const QString &errorText) const;

protected:
    void clear();
    virtual void processHistoryData(QVector<TimeLogEntry> data);
    virtual void processDataInsert(TimeLogEntry data);
    virtual void processDataRemove(const TimeLogEntry &data);
    virtual int findData(const TimeLogEntry &entry) const;
    virtual bool checkStartValid(int indexBefore, int indexAfter, const QDateTime &startTime);

    static bool startTimeCompare(const TimeLogEntry &a, const TimeLogEntry &b);

    TimeTracker *m_timeTracker;
    TimeLogHistory *m_history;
    QVector<TimeLogEntry> m_timeLog;
    QList<qlonglong> m_pendingRequests;
    QList<qlonglong> m_obsoleteRequests;
};

Q_DECLARE_LOGGING_CATEGORY(TIME_LOG_MODEL_CATEGORY)

#endif // TIMELOGMODEL_H
