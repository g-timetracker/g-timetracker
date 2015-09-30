#ifndef TIMELOGMODEL_H
#define TIMELOGMODEL_H

#include <QAbstractListModel>
#include <QSet>

#include <QLoggingCategory>

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

    explicit TimeLogModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    virtual bool removeRows(int row, int count, const QModelIndex &parent);

    TimeLogData timeLogData(const QModelIndex &index) const;
    void appendItem(TimeLogData data = TimeLogData());
    void insertItem(const QModelIndex &index, TimeLogData data = TimeLogData());

private slots:
    void historyError(const QString &errorText);
    void historyDataAvailable(QVector<TimeLogEntry> data, QDateTime until);
    void historyDataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields);

protected:
    void clear();
    virtual void processHistoryData(QVector<TimeLogEntry> data);
    virtual int findData(const TimeLogEntry &entry) const;

    TimeLogHistory *m_history;
    QVector<TimeLogEntry> m_timeLog;
    QSet<QDateTime> m_requestedData;
};

Q_DECLARE_LOGGING_CATEGORY(TIME_LOG_MODEL_CATEGORY)

#endif // TIMELOGMODEL_H
