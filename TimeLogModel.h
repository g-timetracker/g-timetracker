#ifndef TIMELOGMODEL_H
#define TIMELOGMODEL_H

#include <QAbstractListModel>
#include <QSet>

#include "TimeLogEntry.h"
#include "TimeLogHistory.h"

class TimeLogHistory;

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

    explicit TimeLogModel(TimeLogHistory *history, QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual bool canFetchMore(const QModelIndex &parent) const;
    virtual void fetchMore(const QModelIndex & parent);

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    virtual bool removeRows(int row, int count, const QModelIndex &parent);

    TimeLogData timeLogData(const QModelIndex &index) const;
    void appendItem(TimeLogData data = TimeLogData());
    void insertItem(const QModelIndex &index, TimeLogData data = TimeLogData());

signals:
    void error(const QString &errorText) const;

public slots:


private slots:
    void historyError(const QString &errorText);
    void historyDataAvailable(QVector<TimeLogEntry> data, QDateTime limit);
    void historyDataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields);

private:
    void getMoreHistory();

    TimeLogHistory *m_history;
    QVector<TimeLogEntry> m_timeLog;
    QSet<QDateTime> m_requestedData;
};

#endif // TIMELOGMODEL_H
