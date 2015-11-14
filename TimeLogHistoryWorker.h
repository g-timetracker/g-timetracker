#ifndef TIMELOGHISTORYWORKER_H
#define TIMELOGHISTORYWORKER_H

#include <QObject>
#include <QSqlQuery>
#include <QSet>

#include "TimeLogHistory.h"

class TimeLogHistoryWorker : public QObject
{
    Q_OBJECT
public:
    explicit TimeLogHistoryWorker(QObject *parent = 0);
    ~TimeLogHistoryWorker();

    bool init(const QString &dataPath);
    qlonglong size() const;
    QSet<QString> categories() const;

public slots:
    void insert(const TimeLogEntry &data);
    void import(const QVector<TimeLogEntry> &data);
    void remove(const TimeLogEntry &data);
    void edit(const TimeLogEntry &data, TimeLogHistory::Fields fields);
    void editCategory(QString oldName, QString newName);
    void sync(const QVector<TimeLogSyncData> &updatedData,
              const QVector<TimeLogSyncData> &removedData);

    void getHistoryBetween(qlonglong id,
                           const QDateTime &begin = QDateTime::fromTime_t(0),
                           const QDateTime &end = QDateTime::currentDateTime(),
                           const QString &category = QString()) const;
    void getHistoryAfter(qlonglong id, const uint limit,
                         const QDateTime &from = QDateTime::fromTime_t(0)) const;
    void getHistoryBefore(qlonglong id, const uint limit,
                          const QDateTime &until = QDateTime::currentDateTime()) const;

    void getStats(const QDateTime &begin = QDateTime::fromTime_t(0),
                  const QDateTime &end = QDateTime::currentDateTime(),
                  const QString &category = QString(),
                  const QString &separator = ">") const;

    void getSyncData(const QDateTime &mBegin = QDateTime::fromMSecsSinceEpoch(0),
                     const QDateTime &mEnd = QDateTime::currentDateTime()) const;

signals:
    void error(const QString &errorText) const;
    void dataOutdated() const;
    void historyRequestCompleted(QVector<TimeLogEntry> data, qlonglong id) const;
    void dataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields) const;
    void dataInserted(const TimeLogEntry &data) const;
    void dataImported(QVector<TimeLogEntry> data) const;
    void dataRemoved(const TimeLogEntry &data) const;
    void statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const;
    void syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until) const;
    void syncStatsAvailable(QVector<TimeLogSyncData> removedOld, QVector<TimeLogSyncData> removedNew,
                            QVector<TimeLogSyncData> insertedOld, QVector<TimeLogSyncData> insertedNew,
                            QVector<TimeLogSyncData> updatedOld, QVector<TimeLogSyncData> updatedNew) const;
    void dataSynced(QVector<TimeLogSyncData> updatedData, QVector<TimeLogSyncData> removedData);

    void sizeChanged(qlonglong size) const;
    void categoriesChanged(QSet<QString> categories) const;

private:
    bool m_isInitialized;
    QString m_connectionName;
    qlonglong m_size;
    QSet<QString> m_categories;

    bool setupTable();
    bool setupTriggers();
    void setSize(qlonglong size);
    void removeFromCategories(QString category);
    void addToCategories(QString category);

    bool insertData(const QVector<TimeLogEntry> &data);
    bool insertData(const TimeLogSyncData &data);
    bool removeData(const TimeLogSyncData &data);
    bool editData(const TimeLogSyncData &data, TimeLogHistory::Fields fields);
    bool editCategoryData(QString oldName, QString newName);
    bool syncData(const QVector<TimeLogSyncData> &removed, const QVector<TimeLogSyncData> &inserted,
                  const QVector<TimeLogSyncData> &updatedNew, const QVector<TimeLogSyncData> &updatedOld);
    QVector<TimeLogEntry> getHistory(QSqlQuery &query) const;
    QVector<TimeLogStats> getStats(QSqlQuery &query) const;
    QVector<TimeLogSyncData> getSyncData(QSqlQuery &query) const;
    TimeLogEntry getEntry(const QUuid &uuid) const;
    QVector<TimeLogSyncData> getSyncAffected(const QUuid &uuid) const;
    void notifyInsertUpdates(const TimeLogEntry &data) const;
    void notifyInsertUpdates(const QVector<TimeLogEntry> &data) const;
    void notifyRemoveUpdates(const TimeLogEntry &data) const;
    void notifyEditUpdates(const TimeLogEntry &data, TimeLogHistory::Fields fields, QDateTime oldStart = QDateTime()) const;
    void notifyUpdates(const QString &queryString, const QMap<QString, QDateTime> &values,
                       TimeLogHistory::Fields fields = TimeLogHistory::DurationTime | TimeLogHistory::PrecedingStart) const;
    bool updateSize();
    bool updateCategories(const QDateTime &begin = QDateTime::fromTime_t(0),
                          const QDateTime &end = QDateTime::currentDateTime());
};

#endif // TIMELOGHISTORYWORKER_H
