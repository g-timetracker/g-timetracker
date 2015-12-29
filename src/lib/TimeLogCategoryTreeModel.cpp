#include "TimeLogCategoryTreeModel.h"
#include "TimeLogCategory.h"
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

    TimeLogCategory *parentCategory;

    if (!parent.isValid()) {
        parentCategory = m_root.data();
    } else {
        parentCategory = static_cast<TimeLogCategory*>(parent.internalPointer());
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

    TimeLogCategory *childCategory = static_cast<TimeLogCategory*>(child.internalPointer());
    TimeLogCategory *parentCategory = static_cast<TimeLogCategory*>(childCategory->parent());

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

    TimeLogCategory *parentCategory;
    if (!parent.isValid()) {
        parentCategory = m_root.data();
    } else {
        parentCategory = static_cast<TimeLogCategory*>(parent.internalPointer());
    }

    return parentCategory ? parentCategory->children().size() : 0;
}

QVariant TimeLogCategoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    TimeLogCategory *category = static_cast<TimeLogCategory*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return QVariant::fromValue(category->name);
    case FullNameRole:
        return QVariant::fromValue(category->fullName());
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

    return roles;
}

Qt::ItemFlags TimeLogCategoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return SUPER::flags(index) | Qt::ItemIsEditable;
}

void TimeLogCategoryTreeModel::setTimeTracker(TimeTracker *timeTracker)
{
    if (m_timeTracker == timeTracker) {
        return;
    }

    if (m_timeTracker) {
        disconnect(m_timeTracker, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategory>)),
                   this, SLOT(updateCategories(QSharedPointer<TimeLogCategory>)));
    }

    m_timeTracker = timeTracker;

    if (m_timeTracker) {
        connect(m_timeTracker, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategory>)),
                this, SLOT(updateCategories(QSharedPointer<TimeLogCategory>)));
    }

    updateCategories(m_timeTracker ? m_timeTracker->categories() : QSharedPointer<TimeLogCategory>());

    emit timeTrackerChanged(m_timeTracker);
}

void TimeLogCategoryTreeModel::updateCategories(const QSharedPointer<TimeLogCategory> &categories)
{
    beginResetModel();

    m_root = categories;

    endResetModel();
}
