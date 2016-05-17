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

#ifndef TIMELOGCATEGORYTREENODE_H
#define TIMELOGCATEGORYTREENODE_H

#include <QObject>
#include <QVariantMap>
#include <QSharedPointer>

#include "TimeLogCategory.h"

class TimeLogCategoryTreeNode;

class TimeLogCategoryTreeNode
{
public:
    explicit TimeLogCategoryTreeNode(const QString &name, TimeLogCategoryTreeNode *parent = 0);
    ~TimeLogCategoryTreeNode();

    QString fullName() const;
    int depth() const;
    const QMap<QString, TimeLogCategoryTreeNode*> &children() const;

    TimeLogCategoryTreeNode *parent() const;
    void setParent(TimeLogCategoryTreeNode *parent);

    QString name;
    TimeLogCategory category;
    bool hasItems;

private:
    TimeLogCategoryTreeNode *m_parent;
    QMap<QString, TimeLogCategoryTreeNode*> m_children;
};

Q_DECLARE_METATYPE(QSharedPointer<TimeLogCategoryTreeNode>)

#endif // TIMELOGCATEGORYTREENODE_H
