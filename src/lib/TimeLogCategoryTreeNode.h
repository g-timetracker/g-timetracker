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
