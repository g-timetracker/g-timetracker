#ifndef TIMELOGCATEGORY_H
#define TIMELOGCATEGORY_H

#include <QObject>
#include <QMap>
#include <QSharedPointer>

class TimeLogCategory;

class TimeLogCategory
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString fullName READ fullName)
public:
    explicit TimeLogCategory(const QString &category, TimeLogCategory *parent = 0);
    ~TimeLogCategory();

    QString fullName() const;
    int depth() const;
    const QMap<QString, TimeLogCategory*> &children() const;

    TimeLogCategory *parent() const;
    void setParent(TimeLogCategory *parent);

    QString name;

private:
    TimeLogCategory *m_parent;
    QMap<QString, TimeLogCategory*> m_children;
};

Q_DECLARE_METATYPE(QSharedPointer<TimeLogCategory>)

#endif // TIMELOGCATEGORY_H
