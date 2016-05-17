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

#ifndef TIMELOGCATEGORYDEPTHMODEL_H
#define TIMELOGCATEGORYDEPTHMODEL_H

#include <QAbstractListModel>
#include <QSharedPointer>
#include <QRegularExpression>

class TimeLogCategoryTreeNode;
class TimeTracker;
class TimeLogHistory;

class TimeLogCategoryDepthModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(TimeTracker* timeTracker MEMBER m_timeTracker WRITE setTimeTracker NOTIFY timeTrackerChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    typedef QAbstractListModel SUPER;
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        FullNameRole,
        SubcategoriesRole,
        CurrentIndexRole,
        CurrentSubcategoryRole
    };

    TimeLogCategoryDepthModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    void setTimeTracker(TimeTracker *timeTracker);

    QString category() const;
    void setCategory(const QString &category);

signals:
    void timeTrackerChanged(TimeTracker *newTimeTracker);
    void categoryChanged();

private slots:
    void updateCategories(const QSharedPointer<TimeLogCategoryTreeNode> &categories);

private:
    void setSubcategory(int level, const QString &subcategory);
    void setCategoryFields(int rowIndex, const QStringList &categoryFields);

    TimeTracker *m_timeTracker;
    QSharedPointer<TimeLogCategoryTreeNode> m_root;
    QStringList m_categoryFields;
    QList<TimeLogCategoryTreeNode*> m_categoryEntries;
    QRegularExpression m_splitRegexp;
};

#endif // TIMELOGCATEGORYDEPTHMODEL_H
