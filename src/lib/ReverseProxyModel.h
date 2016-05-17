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

#ifndef REVERSEPROXYMODEL_H
#define REVERSEPROXYMODEL_H

#include <QAbstractProxyModel>

#include "TimeLogData.h"

class ReverseProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
    typedef QAbstractProxyModel SUPER;
public:
    explicit ReverseProxyModel(QObject* parent = 0);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int columnCount(const QModelIndex &parent) const;
    virtual int rowCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &proxyIndex, int role) const;

    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;

    virtual void setSourceModel(QAbstractItemModel* newSourceModel);

    Q_INVOKABLE void removeItem(const QModelIndex &index);
    Q_INVOKABLE void appendItem(TimeLogData data = TimeLogData());
    Q_INVOKABLE void insertItem(const QModelIndex &index, TimeLogData data = TimeLogData());

private slots:
    void sourceRowsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end);
    void sourceRowsInserted(const QModelIndex &sourceParent, int first, int last);
    void sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &sourceParent, int first, int last);
    void sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void sourceLayoutChanged(const QList<QPersistentModelIndex> & sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void sourceAboutToBeReset();
    void sourceReset();
    void sourceDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight, const QVector<int> &roles);

private:
    QModelIndexList m_proxyIndexes;
    QList<QPersistentModelIndex> m_sourceIndexes;
};

#endif // REVERSEPROXYMODEL_H
