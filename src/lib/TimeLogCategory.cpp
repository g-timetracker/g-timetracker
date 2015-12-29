#include "TimeLogCategory.h"

TimeLogCategory::TimeLogCategory(const QString &category, TimeLogCategory *parent) :
    name(category),
    m_parent(nullptr)
{
    if (parent) {
        setParent(parent);
    }
}

TimeLogCategory::~TimeLogCategory()
{
    if (m_parent) {
        m_parent->m_children.remove(name);
    }

    qDeleteAll(m_children.values());
}

QString TimeLogCategory::fullName() const
{
    QString result(name);

    for (TimeLogCategory *category = m_parent; category && category->m_parent;
         category = category->m_parent) {
        result.prepend(QString("%1 > ").arg(category->name));
    }

    return result;
}

int TimeLogCategory::depth() const
{
    int result = 0;

    const TimeLogCategory *category = this;
    while ((category = category->m_parent)) {
        ++result;
    }

    return result;
}

const QMap<QString, TimeLogCategory *> &TimeLogCategory::children() const
{
    return m_children;
}

TimeLogCategory *TimeLogCategory::parent() const
{
    return m_parent;
}

void TimeLogCategory::setParent(TimeLogCategory *parent)
{
    if (m_parent) {
        m_parent->m_children.remove(name);
    }

    m_parent = parent;

    if (parent) {
        m_parent->m_children.insert(name, this);
    }
}
