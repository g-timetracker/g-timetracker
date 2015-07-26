#ifndef TIMELOGMODEL_H
#define TIMELOGMODEL_H

#include <QAbstractListModel>

#include "TimeLogEntry.h"

class TimeLogModel : public QAbstractListModel
{
    Q_OBJECT
    typedef QAbstractListModel SUPER;
public:
    enum Roles {
        StartTimeRole = Qt::UserRole + 1,
        DurationTimeRole,
        CategoryRole,
        CommentRole
    };

    explicit TimeLogModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

signals:

public slots:
    void appendItem(TimeLogData data = TimeLogData());
    void insertItem(const QModelIndex &index, TimeLogData data = TimeLogData());
    void removeItem(const QModelIndex &index);

private slots:
    void processRowsInserted(const QModelIndex &parent, int first, int last);
    void processRowsRemoved(const QModelIndex &parent, int first, int last);

private:
    QList<TimeLogEntry> m_timeLog;
};

#endif // TIMELOGMODEL_H
