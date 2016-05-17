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

#include "TimeLogCategoryTreeModel.h"
#include "TimeLogCategoryTreeNode.h"
#include "TimeTracker.h"

TimeLogCategoryTreeModel::TimeLogCategoryTreeModel(QObject *parent) :
    SUPER(parent),
    m_timeTracker(nullptr)
{

}

QModelIndex TimeLogCategoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TimeLogCategoryTreeNode *parentCategory;

    if (!parent.isValid()) {
        parentCategory = m_root.data();
    } else {
        parentCategory = static_cast<TimeLogCategoryTreeNode*>(parent.internalPointer());
    }

    if (parentCategory && row < parentCategory->children().size()) {
        return createIndex(row, column,
                           parentCategory->children().value(parentCategory->children().keys().at(row)));
    } else {
        return QModelIndex();
    }
}

QModelIndex TimeLogCategoryTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    TimeLogCategoryTreeNode *childCategory = static_cast<TimeLogCategoryTreeNode*>(child.internalPointer());
    TimeLogCategoryTreeNode *parentCategory = static_cast<TimeLogCategoryTreeNode*>(childCategory->parent());

    if (parentCategory == m_root) {
        return QModelIndex();
    }

    int row = parentCategory->parent() ? parentCategory->parent()->children().keys().indexOf(parentCategory->name) : 0;
    return createIndex(row, 0, parentCategory);
}

int TimeLogCategoryTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 1;
}

int TimeLogCategoryTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    TimeLogCategoryTreeNode *parentCategory;
    if (!parent.isValid()) {
        parentCategory = m_root.data();
    } else {
        parentCategory = static_cast<TimeLogCategoryTreeNode*>(parent.internalPointer());
    }

    return parentCategory ? parentCategory->children().size() : 0;
}

QVariant TimeLogCategoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    TimeLogCategoryTreeNode *node = static_cast<TimeLogCategoryTreeNode*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return QVariant::fromValue(node->name);
    case FullNameRole:
        return QVariant::fromValue(node->fullName());
    case DataRole:
        return QVariant::fromValue(node->category.data);
    case CommentRole:
        return QVariant::fromValue(node->category.data.value("comment"));
    case HasItemsRole:
        return QVariant::fromValue(node->hasItems);
    case CategoryRole:
        return QVariant::fromValue(TimeLogCategoryData(node->category));
    default:
        return QVariant();
    }
}

QVariant TimeLogCategoryTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return QString("Category");
    } else {
        return section;
    }
}

QHash<int, QByteArray> TimeLogCategoryTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles = SUPER::roleNames();
    roles[NameRole] = "name";
    roles[FullNameRole] = "fullName";
    roles[DataRole] = "data";
    roles[CommentRole] = "comment";
    roles[HasItemsRole] = "hasItems";
    roles[CategoryRole] = "category";

    return roles;
}

Qt::ItemFlags TimeLogCategoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return SUPER::flags(index) | Qt::ItemIsEditable;
}

bool TimeLogCategoryTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!m_timeTracker) {
        return false;
    }

    if (!index.isValid()) {
        return false;
    }

    if (role != CategoryRole) {
        return false;
    }

    TimeLogCategoryData data = value.value<TimeLogCategoryData>();
    Q_ASSERT(data.isValid());
    TimeLogCategoryTreeNode *node = static_cast<TimeLogCategoryTreeNode*>(index.internalPointer());
    TimeLogCategory category(node->category.uuid, data);
    if (category.uuid.isNull()) {
        category.uuid = QUuid::createUuid();
    }

    m_timeTracker->editCategory(node->category.name, category);

    return true;
}

void TimeLogCategoryTreeModel::removeItem(const QModelIndex &index)
{
    if (!m_timeTracker) {
        return;
    }

    if (!index.isValid()) {
        return;
    }

    TimeLogCategoryTreeNode *node = static_cast<TimeLogCategoryTreeNode*>(index.internalPointer());

    m_timeTracker->removeCategory(node->category.name);
}

void TimeLogCategoryTreeModel::removeItem(const QString &name)
{
    m_timeTracker->removeCategory(name);
}

void TimeLogCategoryTreeModel::addItem(TimeLogCategoryData data)
{
    if (!m_timeTracker) {
        return;
    }

    m_timeTracker->addCategory(TimeLogCategory(QUuid::createUuid(), data));
}

void TimeLogCategoryTreeModel::setTimeTracker(TimeTracker *timeTracker)
{
    if (m_timeTracker == timeTracker) {
        return;
    }

    if (m_timeTracker) {
        disconnect(m_timeTracker, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)),
                   this, SLOT(updateCategories(QSharedPointer<TimeLogCategoryTreeNode>)));
    }

    m_timeTracker = timeTracker;

    if (m_timeTracker) {
        connect(m_timeTracker, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)),
                this, SLOT(updateCategories(QSharedPointer<TimeLogCategoryTreeNode>)));
    }

    updateCategories(m_timeTracker ? m_timeTracker->categories()
                                   : QSharedPointer<TimeLogCategoryTreeNode>());

    emit timeTrackerChanged(m_timeTracker);
}

void TimeLogCategoryTreeModel::updateCategories(const QSharedPointer<TimeLogCategoryTreeNode> &categories)
{
    beginResetModel();

    m_root = categories;

    endResetModel();
}
