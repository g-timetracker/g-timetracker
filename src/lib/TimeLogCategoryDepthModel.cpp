#include "TimeLogCategoryDepthModel.h"
#include "TimeLogCategoryTreeNode.h"
#include "TimeTracker.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CATEGORY_DEPTH_CATEGORY, "TimeLogCategoryDepthModel", QtInfoMsg)

const QString categorySplitPattern("\\s*>\\s*");

TimeLogCategoryDepthModel::TimeLogCategoryDepthModel(QObject *parent) :
    SUPER(parent),
    m_timeTracker(nullptr),
    m_splitRegexp(categorySplitPattern)
{
}

int TimeLogCategoryDepthModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (!m_root) {
        return 0;
    }

    const TimeLogCategoryTreeNode *current;
    if (m_categoryFields.isEmpty() ) {
        current = m_root.data();
    } else {
        current = m_categoryEntries.size() == m_categoryFields.size() ? m_categoryEntries.constLast()
                                                                      : nullptr;
    }

    int result = m_categoryFields.size() + ((!current || current->children().isEmpty()) ? 0 : 1);
    qCDebug(CATEGORY_DEPTH_CATEGORY) << result;
    return m_categoryFields.size() + ((!current || current->children().isEmpty()) ? 0 : 1);
}

QVariant TimeLogCategoryDepthModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return QVariant::fromValue(m_categoryFields.at(index.row()));
    case FullNameRole:
        return QVariant::fromValue(m_categoryFields.mid(0, index.row() + 1).join(" > "));
    case SubcategoriesRole:
        if (index.row() == 0) {
            qCDebug(CATEGORY_DEPTH_CATEGORY) << index.row() << "subcategories (root):" << m_root->children().keys();
            return QVariant::fromValue(m_root->children().keys());
        } else if (m_categoryEntries.size() > index.row() - 1) {
            qCDebug(CATEGORY_DEPTH_CATEGORY) << index.row() << "subcategories:" << m_categoryEntries.at(index.row() - 1)->children().keys();
            return QVariant::fromValue(m_categoryEntries.at(index.row() - 1)->children().keys());
        } else {
            qCDebug(CATEGORY_DEPTH_CATEGORY) << index.row() << "subcategories: empty";
            return QVariant();
        }
    case CurrentSubcategoryRole:
        if (m_categoryFields.size() > index.row()) {
            return QVariant::fromValue(m_categoryFields.at(index.row()));
        } else {
            return QVariant::fromValue(QString(""));
        }
    case CurrentIndexRole: {
        TimeLogCategoryTreeNode *category;
        if (index.row() == 0) {
            category = m_root.data();
        } else if (m_categoryEntries.size() >= index.row()) {
            category = m_categoryEntries.at(index.row() - 1);
        } else {
            category = nullptr;
        }
        if (!category || m_categoryFields.size() <= index.row()) {
            qCDebug(CATEGORY_DEPTH_CATEGORY) << index.row() << "current index:" << 0 << category << m_categoryFields;
            return QVariant::fromValue(0);
        } else {
            qCDebug(CATEGORY_DEPTH_CATEGORY) << index.row() << "current index:" << category->children().keys().indexOf(m_categoryFields.at(index.row())) + 1;
            return QVariant::fromValue(category->children().keys().indexOf(m_categoryFields.at(index.row())) + 1);
        }
    }
    default:
        return QVariant();
    }
}

QVariant TimeLogCategoryDepthModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return QString("data");
    } else {
        return section;
    }
}

QHash<int, QByteArray> TimeLogCategoryDepthModel::roleNames() const
{
    QHash<int, QByteArray> roles = SUPER::roleNames();
    roles[NameRole] = "name";
    roles[FullNameRole] = "fullName";
    roles[SubcategoriesRole] = "subcategories";
    roles[CurrentIndexRole] = "subcategoryIndex";
    roles[CurrentSubcategoryRole] = "currentSubcategory";

    return roles;
}

Qt::ItemFlags TimeLogCategoryDepthModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return SUPER::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool TimeLogCategoryDepthModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qCDebug(CATEGORY_DEPTH_CATEGORY) << index.row() << value << role;

    if (!index.isValid()) {
        return false;
    }

    switch (role) {
    case CurrentSubcategoryRole: {
        QString subcategoryName = value.toString();
        setSubcategory(index.row(), subcategoryName);
        break;
    }
    case CurrentIndexRole: {
        bool ok;
        int newIndex = value.toInt(&ok);
        if (!ok) {
            return false;
        }
        QString subcategoryName;
        if (newIndex <= 0) {
            subcategoryName = QString("");
        } else if (index.row() == 0) {
            subcategoryName = m_root->children().keys().at(newIndex - 1);
        } else if (m_categoryEntries.size() > index.row() - 1) {
            subcategoryName = m_categoryEntries.at(index.row() - 1)->children().keys().at(newIndex - 1);
        } else {
            subcategoryName = QString("");
        }
        setSubcategory(index.row(), subcategoryName);
        break;
    }
    default:
        return false;
    }

    return true;
}

void TimeLogCategoryDepthModel::setTimeTracker(TimeTracker *timeTracker)
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

QString TimeLogCategoryDepthModel::category() const
{
    return m_categoryFields.join(" > ");
}

void TimeLogCategoryDepthModel::setCategory(const QString &category)
{
    QStringList categoryFields = category.split(m_splitRegexp, QString::SkipEmptyParts);

    int startIndex = categoryFields.size();
    for (int i = 0; i < categoryFields.size(); i++) {
        if (m_categoryFields.size() <= i || m_categoryFields.at(i) != categoryFields.at(i)) {
            startIndex = i;
            break;
        }
    }

    if (startIndex == categoryFields.size() && startIndex == m_categoryFields.size()) {
        qCDebug(CATEGORY_DEPTH_CATEGORY) << "Same category fields" << categoryFields << m_categoryFields;
        return;
    }

    setCategoryFields(startIndex, categoryFields.mid(startIndex));
}

void TimeLogCategoryDepthModel::updateCategories(const QSharedPointer<TimeLogCategoryTreeNode> &categories)
{
    beginResetModel();

    m_root = categories;

    QStringList categoryFields(m_categoryFields);

    m_categoryEntries.clear();
    m_categoryFields.clear();

    endResetModel();

    setCategoryFields(0, categoryFields);
}

void TimeLogCategoryDepthModel::setSubcategory(int level, const QString &subcategory)
{
    setCategoryFields(level, subcategory.isEmpty() ? QStringList() : QStringList() << subcategory);
}

void TimeLogCategoryDepthModel::setCategoryFields(int startLevel, const QStringList &categoryFields)
{
    if (!m_root) {
        return;
    }
    if (startLevel > m_categoryFields.size()) {
        qCCritical(CATEGORY_DEPTH_CATEGORY) << "Start level higher than current size"
                                            << startLevel << m_categoryFields.size();
        return;
    }

    qCDebug(CATEGORY_DEPTH_CATEGORY) << "begin" << m_categoryFields << "=>" << startLevel
                                     << categoryFields << m_categoryEntries;

    int oldSize = m_categoryFields.size();
    int newSize = startLevel + categoryFields.size();
    int startRow = qMin(startLevel, oldSize);

    TimeLogCategoryTreeNode *parentCategory;
    if (startRow == 0) {
        parentCategory = m_root.data();
    } else {
        parentCategory = m_categoryEntries.size() >= startRow ? m_categoryEntries.at(startRow - 1) : nullptr;
    }
    TimeLogCategoryTreeNode *category = parentCategory;
    for (int i = 0; i < categoryFields.size() && category && category->depth() < newSize; i++) {
        category = category->children().value(categoryFields.at(i));
    }

    TimeLogCategoryTreeNode *current;
    if (m_categoryFields.isEmpty() ) {
        current = m_root.data();
    } else {
        current = m_categoryEntries.size() == m_categoryFields.size() ? m_categoryEntries.constLast()
                                                                      : nullptr;
    }

    bool oldHasChildren = current && !current->children().isEmpty();
    bool newHasChildren = category && !category->children().isEmpty();
    int oldRows = rowCount(QModelIndex());
    int newRows = newSize + (newHasChildren ? 1 : 0);

    int removeStart = startRow + 1;
    int removeEnd = oldRows - 1;

    int insertStart = startRow + 1;
    int insertEnd = newRows - 1;

    bool isItemsEdit = true;
    bool isItemsInsert = insertStart <= insertEnd;
    bool isItemsRemove = removeStart <= removeEnd;

    if (parentCategory && !categoryFields.isEmpty()) {
        category = parentCategory->children().value(categoryFields.constFirst());
    }

    if (isItemsRemove) {
        qCDebug(CATEGORY_DEPTH_CATEGORY) << "removing:" << removeStart << removeEnd
                                         << oldHasChildren << newHasChildren;
        beginRemoveRows(QModelIndex(), removeStart, removeEnd);
        while (m_categoryFields.size() > removeStart) {
            m_categoryFields.removeAt(removeStart);
        }
        while (m_categoryEntries.size() > removeStart) {
            m_categoryEntries.removeAt(removeStart);
        }
        endRemoveRows();
    }

    for (int i = 0; i < categoryFields.size() && category && category->depth() < startRow + 1; i++) {
        category = category->children().value(categoryFields.at(i));
    }

    if (isItemsEdit) {
        if (!categoryFields.isEmpty()) {
            if (m_categoryFields.size() > startRow) {
                m_categoryFields[startRow] = categoryFields.constFirst();
            } else {
                m_categoryFields.append(categoryFields.constFirst());
            }
            if (category) {
                if (m_categoryEntries.size() > startRow) {
                    m_categoryEntries[startRow] = category;
                } else {
                    m_categoryEntries.append(category);
                }
            }
        } else {
            if (m_categoryFields.size() == startRow + 1) {
                m_categoryFields.removeAt(startRow);
            }
            if (m_categoryEntries.size() == startRow + 1) {
                m_categoryEntries.removeAt(startRow);
            }
        }

        qCDebug(CATEGORY_DEPTH_CATEGORY) << "editing current:" << startRow << startRow
                                         << oldHasChildren << newHasChildren;

        emit dataChanged(index(startRow, 0, QModelIndex()),
                         index(startRow, 0, QModelIndex()),
                         QVector<int>() << CurrentIndexRole);
    }

    if (isItemsInsert) {
        qCDebug(CATEGORY_DEPTH_CATEGORY) << "inserting:" << insertStart << insertEnd
                                         << oldHasChildren << newHasChildren;

        beginInsertRows(QModelIndex(), insertStart, insertEnd);

        for (int i = 1; i + startRow < newSize; i++) {
            if (category) {
                category = category->children().value(categoryFields.at(i));
                if (category) {
                    m_categoryEntries.append(category);
                }
            }
            m_categoryFields.append(categoryFields.at(i));
        }
        endInsertRows();
    }

    qCDebug(CATEGORY_DEPTH_CATEGORY) << "end" << m_categoryFields << "=>" << startLevel
                                     << categoryFields << m_categoryEntries;

    emit categoryChanged();
}
