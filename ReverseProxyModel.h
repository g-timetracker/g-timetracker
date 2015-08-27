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

    Q_INVOKABLE TimeLogData timeLogData(const QModelIndex &index) const;
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
