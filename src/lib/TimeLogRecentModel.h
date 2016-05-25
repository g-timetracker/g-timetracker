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

#ifndef TIMELOGRECENTMODEL_H
#define TIMELOGRECENTMODEL_H

#include "TimeLogModel.h"

class TimeLogRecentModel : public TimeLogModel
{
    Q_OBJECT
    Q_PROPERTY(qlonglong availableSize MEMBER m_availableSize WRITE setAvailableSize NOTIFY availableSizeChanged)
    typedef TimeLogModel SUPER;
public:
    explicit TimeLogRecentModel(QObject *parent = 0);

    virtual bool canFetchMore(const QModelIndex &parent) const;
    virtual void fetchMore(const QModelIndex & parent);

signals:
    void availableSizeChanged(qlonglong newAvailableSize);

protected:
    virtual void processHistoryData(QVector<TimeLogEntry> data);
    virtual void processDataInsert(TimeLogEntry data);
    virtual int findData(const TimeLogEntry &entry) const;

protected slots:
    virtual void setHistory(TimeLogHistory *history);

private slots:
    void setAvailableSize(qlonglong availableSize);

private:
    void getMoreHistory();

    mutable bool m_moreDataRequested;
    qlonglong m_availableSize;
};

#endif // TIMELOGRECENTMODEL_H
