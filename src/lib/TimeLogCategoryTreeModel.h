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

#ifndef TIMELOGCATEGORYTREEMODEL_H
#define TIMELOGCATEGORYTREEMODEL_H

#include <QAbstractItemModel>
#include <QSharedPointer>

#include "TimeLogCategoryData.h"

class TimeLogCategoryTreeNode;
class TimeTracker;

class TimeLogCategoryTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(TimeTracker* timeTracker MEMBER m_timeTracker WRITE setTimeTracker NOTIFY timeTrackerChanged)
    typedef QAbstractItemModel SUPER;
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        FullNameRole,
        DataRole,
        CommentRole,
        HasItemsRole,
        CategoryRole
    };
    Q_ENUM(Roles)

    explicit TimeLogCategoryTreeModel(QObject *parent = 0);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int columnCount(const QModelIndex &parent) const;
    virtual int rowCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    Q_INVOKABLE void removeItem(const QModelIndex &index);
    Q_INVOKABLE void removeItem(const QString &name);
    Q_INVOKABLE void addItem(TimeLogCategoryData data);

    void setTimeTracker(TimeTracker *timeTracker);

signals:
    void timeTrackerChanged(TimeTracker *newTimeTracker);

private slots:
    void updateCategories(const QSharedPointer<TimeLogCategoryTreeNode> &categories);

private:
    TimeTracker *m_timeTracker;
    QSharedPointer<TimeLogCategoryTreeNode> m_root;
};

#endif // TIMELOGCATEGORYTREEMODEL_H
