#include "ReverseProxyModel.h"
#include "TimeLogModel.h"

ReverseProxyModel::ReverseProxyModel(QObject* parent) :
    SUPER(parent)
{

}

QModelIndex ReverseProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    const QModelIndex sourceParent = mapToSource(parent);
    const QModelIndex sourceIndex = sourceModel()->index(sourceModel()->rowCount(sourceParent) - 1 - row,
                                                         column, sourceParent);

    return mapFromSource(sourceIndex);
}

QModelIndex ReverseProxyModel::parent(const QModelIndex &child) const
{
    const QModelIndex sourceIndex = mapToSource(child);
    const QModelIndex sourceParent = sourceIndex.parent();

    return mapFromSource(sourceParent);
}

int ReverseProxyModel::columnCount(const QModelIndex &parent) const
{
    return sourceModel()->columnCount(mapToSource(parent));
}

int ReverseProxyModel::rowCount(const QModelIndex &parent) const
{
    return sourceModel()->rowCount(mapToSource(parent));
}

QVariant ReverseProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    return sourceModel()->data(mapToSource(proxyIndex), role);
}

QModelIndex ReverseProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceModel() || !sourceIndex.isValid()) {
        return QModelIndex();
    }

    return createIndex(sourceModel()->rowCount(QModelIndex()) - 1 - sourceIndex.row(),
                       sourceIndex.column(), sourceIndex.internalPointer());
}

QModelIndex ReverseProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid()) {
        return QModelIndex();
    }

    return sourceModel()->index(sourceModel()->rowCount(QModelIndex()) - 1 - proxyIndex.row(),
                                proxyIndex.column(), QModelIndex());
}

void ReverseProxyModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    if (sourceModel() == newSourceModel)
        return;

    beginResetModel();

    if (sourceModel()) {
        disconnect(sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        disconnect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
        disconnect(sourceModel(), SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
                   this, SLOT(sourceLayoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
        disconnect(sourceModel(), SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
                   this, SLOT(sourceLayoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
        disconnect(sourceModel(), SIGNAL(modelAboutToBeReset()),
                   this, SLOT(sourceAboutToBeReset()));
        disconnect(sourceModel(), SIGNAL(modelReset()),
                   this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
                   this, SLOT(sourceDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    }

    SUPER::setSourceModel(newSourceModel);

    if (newSourceModel) {
        connect(newSourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        connect(newSourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        connect(newSourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(newSourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
        connect(newSourceModel, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
                this, SLOT(sourceLayoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
        connect(newSourceModel, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
                this, SLOT(sourceLayoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
        connect(newSourceModel, SIGNAL(modelAboutToBeReset()),
                this, SLOT(sourceAboutToBeReset()));
        connect(newSourceModel, SIGNAL(modelReset()),
                this, SLOT(sourceReset()));
        connect(newSourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
                this, SLOT(sourceDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    }

    endResetModel();
}

void ReverseProxyModel::removeItem(const QModelIndex &index)
{
    QModelIndex sourceIndex = mapToSource(index);

    sourceModel()->removeRow(sourceIndex.row(), sourceIndex.parent());
}

void ReverseProxyModel::appendItem(TimeLogData data)
{
    if (!sourceModel()) {
        return;
    }

    TimeLogModel *model = static_cast<TimeLogModel*>(sourceModel());
    model->appendItem(data);
}

void ReverseProxyModel::insertItem(const QModelIndex &index, TimeLogData data)
{
    if (!sourceModel()) {
        return;
    }

    TimeLogModel *model = static_cast<TimeLogModel*>(sourceModel());
    model->insertItem(mapToSource(index), data);
}

void ReverseProxyModel::sourceRowsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end)
{
    int newRows = sourceModel()->rowCount(sourceParent) + (end - start + 1);
    beginInsertRows(mapFromSource(sourceParent), newRows - 1 - end, newRows - 1 - start);
}

void ReverseProxyModel::sourceRowsInserted(const QModelIndex &sourceParent, int first, int last)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    endInsertRows();
}

void ReverseProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
    int sourceRows = sourceModel()->rowCount(sourceParent);
    beginRemoveRows(mapFromSource(sourceParent), sourceRows - 1 - end, sourceRows - 1 - start);
}

void ReverseProxyModel::sourceRowsRemoved(const QModelIndex &sourceParent, int first, int last)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    endRemoveRows();
}

void ReverseProxyModel::sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, LayoutChangeHint hint)
{
    QModelIndexList proxyPersistentIndexes = persistentIndexList();
    foreach(const QPersistentModelIndex &proxyPersistentIndex, proxyPersistentIndexes) {
        m_proxyIndexes << proxyPersistentIndex;
        m_sourceIndexes << mapToSource(proxyPersistentIndex);
    }

    QList<QPersistentModelIndex> proxyParents;
    proxyParents.reserve(sourceParents.size());
    foreach (const QPersistentModelIndex &parent, sourceParents) {
        QPersistentModelIndex proxyParent;
        if (parent.isValid()) {
            proxyParent = mapFromSource(parent);
        }
        proxyParents << proxyParent;
    }

    layoutAboutToBeChanged(proxyParents, hint);
}

void ReverseProxyModel::sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, LayoutChangeHint hint)
{
    QModelIndexList sourceIndexes;
    sourceIndexes.reserve(m_sourceIndexes.size());
    for (int i = 0; i < m_sourceIndexes.size(); ++i) {
        sourceIndexes << mapFromSource(m_sourceIndexes.at(i));
    }
    changePersistentIndexList(m_proxyIndexes, sourceIndexes);

    m_sourceIndexes.clear();
    m_proxyIndexes.clear();

    QList<QPersistentModelIndex> proxyParents;
    proxyParents.reserve(sourceParents.size());
    foreach (const QPersistentModelIndex &parent, sourceParents) {
        QPersistentModelIndex proxyParent;
        if (parent.isValid()) {
            proxyParent = mapFromSource(parent);
        }
        proxyParents << proxyParent;
    }

    layoutChanged(proxyParents, hint);
}

void ReverseProxyModel::sourceAboutToBeReset()
{
    beginResetModel();
}

void ReverseProxyModel::sourceReset()
{
    endResetModel();
}

void ReverseProxyModel::sourceDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight, const QVector<int> &roles)
{
    emit dataChanged(mapFromSource(sourceBottomRight), mapFromSource(sourceTopLeft), roles);
}
