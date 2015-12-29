#ifndef TIMELOGCATEGORYDEPTHMODEL_H
#define TIMELOGCATEGORYDEPTHMODEL_H

#include <QAbstractListModel>
#include <QSharedPointer>
#include <QRegularExpression>

class TimeLogCategory;
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
    void updateCategories(const QSharedPointer<TimeLogCategory> &categories);

private:
    void setSubcategory(int level, const QString &subcategory);
    void setCategoryFields(int rowIndex, const QStringList &categoryFields);

    TimeTracker *m_timeTracker;
    QSharedPointer<TimeLogCategory> m_root;
    QStringList m_categoryFields;
    QList<TimeLogCategory*> m_categoryEntries;
    QRegularExpression m_splitRegexp;
};

#endif // TIMELOGCATEGORYDEPTHMODEL_H
