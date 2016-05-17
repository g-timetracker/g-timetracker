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

#include "TimeLogCategoryTreeNode.h"

TimeLogCategoryTreeNode::TimeLogCategoryTreeNode(const QString &name, TimeLogCategoryTreeNode *parent) :
    name(name),
    hasItems(false),
    m_parent(nullptr)
{
    if (parent) {
        setParent(parent);
    }
}

TimeLogCategoryTreeNode::~TimeLogCategoryTreeNode()
{
    if (m_parent) {
        m_parent->m_children.remove(name);
    }

    qDeleteAll(m_children.values());
}

QString TimeLogCategoryTreeNode::fullName() const
{
    QString result(name);

    for (TimeLogCategoryTreeNode *node = m_parent; node && node->m_parent;
         node = node->m_parent) {
        result.prepend(QString("%1 > ").arg(node->name));
    }

    return result;
}

int TimeLogCategoryTreeNode::depth() const
{
    int result = 0;

    const TimeLogCategoryTreeNode *node = this;
    while ((node = node->m_parent)) {
        ++result;
    }

    return result;
}

const QMap<QString, TimeLogCategoryTreeNode *> &TimeLogCategoryTreeNode::children() const
{
    return m_children;
}

TimeLogCategoryTreeNode *TimeLogCategoryTreeNode::parent() const
{
    return m_parent;
}

void TimeLogCategoryTreeNode::setParent(TimeLogCategoryTreeNode *parent)
{
    if (m_parent) {
        m_parent->m_children.remove(name);
    }

    m_parent = parent;

    if (parent) {
        m_parent->m_children.insert(name, this);
    }
}
