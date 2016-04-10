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
