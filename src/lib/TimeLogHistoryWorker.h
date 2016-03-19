#ifndef TIMELOGHISTORYWORKER_H
#define TIMELOGHISTORYWORKER_H

#include <QObject>
#include <QSqlQuery>
#include <QSet>
#include <QStack>
#include <QSharedPointer>
#include <QRegularExpression>

#include "TimeLogHistory.h"

class TimeLogCategory;

class TimeLogHistoryWorker : public QObject
{
    Q_OBJECT
public:
    explicit TimeLogHistoryWorker(QObject *parent = 0);
    ~TimeLogHistoryWorker();

    bool init(const QString &dataPath, const QString &filePath = QString());
    qlonglong size() const;
    QSharedPointer<TimeLogCategory> categories() const;

public slots:
    void insert(const TimeLogEntry &data);
    void import(const QVector<TimeLogEntry> &data);
    void remove(const TimeLogEntry &data);
    void edit(const TimeLogEntry &data, TimeLogHistory::Fields fields);
    void editCategory(QString oldName, QString newName);
    void sync(const QVector<TimeLogSyncData> &updatedData,
              const QVector<TimeLogSyncData> &removedData);
    void updateHashes();

    void undo();

    void getHistoryBetween(qlonglong id,
                           const QDateTime &begin = QDateTime::fromTime_t(0, Qt::UTC),
                           const QDateTime &end = QDateTime::currentDateTimeUtc(),
                           const QString &category = QString()) const;
    void getHistoryAfter(qlonglong id, const uint limit,
                         const QDateTime &from = QDateTime::fromTime_t(0, Qt::UTC)) const;
    void getHistoryBefore(qlonglong id, const uint limit,
                          const QDateTime &until = QDateTime::currentDateTimeUtc()) const;

    void getStats(const QDateTime &begin = QDateTime::fromTime_t(0, Qt::UTC),
                  const QDateTime &end = QDateTime::currentDateTimeUtc(),
                  const QString &category = QString(),
                  const QString &separator = ">") const;

    void getSyncData(const QDateTime &mBegin = QDateTime(),
                     const QDateTime &mEnd = QDateTime()) const;
    void getSyncDataSize(const QDateTime &mBegin = QDateTime::fromMSecsSinceEpoch(0, Qt::UTC),
                         const QDateTime &mEnd = QDateTime::currentDateTimeUtc()) const;

    void getHashes(const QDateTime &maxDate = QDateTime(), bool noUpdate = false);

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
    void syncDataSizeAvailable(qlonglong size, QDateTime mBegin, QDateTime mEnd) const;
    void syncStatsAvailable(QVector<TimeLogSyncData> removedOld, QVector<TimeLogSyncData> removedNew,
                            QVector<TimeLogSyncData> insertedOld, QVector<TimeLogSyncData> insertedNew,
                            QVector<TimeLogSyncData> updatedOld, QVector<TimeLogSyncData> updatedNew) const;
    void hashesAvailable(QMap<QDateTime, QByteArray> hashes) const;
    void dataSynced(QVector<TimeLogSyncData> updatedData, QVector<TimeLogSyncData> removedData);
    void hashesUpdated() const;

    void sizeChanged(qlonglong size) const;
    void categoriesChanged(QSharedPointer<TimeLogCategory> categories) const;
    void undoCountChanged(int undoCount) const;

private:
    class Undo
    {
    public:
        enum Type {
            Insert,
            Remove,
            Edit,
            EditCategory
        };

        Type type;
        QVector<TimeLogEntry> data;
        QVector<TimeLogHistory::Fields> fields;
    };

    bool m_isInitialized;
    QString m_connectionName;
    qlonglong m_size;
    QSet<QString> m_categorySet;
    QSharedPointer<TimeLogCategory> m_categoryTree;
    QRegularExpression m_categorySplitRegexp;
    QStack<Undo> m_undoStack;

    QSqlQuery *m_insertQuery;
    QSqlQuery *m_removeQuery;
    QSqlQuery *m_notifyInsertQuery;
    QSqlQuery *m_notifyRemoveQuery;
    QSqlQuery *m_notifyEditQuery;
    QSqlQuery *m_notifyEditStartQuery;
    QSqlQuery *m_syncAffectedQuery;
    QSqlQuery *m_entryQuery;

    bool setupTable();
    bool setupTriggers();
    void setSize(qlonglong size);
    void removeFromCategories(QString category);
    void addToCategories(QString category);
    void processFail();

    void insertEntry(const TimeLogEntry &data);
    void removeEntry(const TimeLogEntry &data);
    bool editEntry(const TimeLogEntry &data, TimeLogHistory::Fields fields);
    void editEntries(const QVector<TimeLogEntry> &data, const QVector<TimeLogHistory::Fields> &fields);

    bool insertData(const QVector<TimeLogEntry> &data);
    bool insertData(const TimeLogSyncData &data);
    bool removeData(const TimeLogSyncData &data);
    bool editData(const TimeLogSyncData &data, TimeLogHistory::Fields fields);
    bool editCategoryData(QString oldName, QString newName);
    bool syncData(const QVector<TimeLogSyncData> &removed, const QVector<TimeLogSyncData> &inserted,
                  const QVector<TimeLogSyncData> &updatedNew, const QVector<TimeLogSyncData> &updatedOld);
    void updateDataHashes(const QMap<QDateTime, QByteArray> &hashes);
    bool writeHash(const QDateTime &start, const QByteArray &hash);
    bool removeHash(const QDateTime &start);
    QVector<TimeLogEntry> getHistory(QSqlQuery &query) const;
    QVector<TimeLogStats> getStats(QSqlQuery &query) const;
    QVector<TimeLogSyncData> getSyncData(QSqlQuery &query) const;
    TimeLogEntry getEntry(const QUuid &uuid);
    QVector<TimeLogEntry> getEntries(const QString &category) const;
    QVector<TimeLogSyncData> getSyncAffected(const QUuid &uuid);
    QMap<QDateTime, QByteArray> getDataHashes(const QDateTime &maxDate = QDateTime()) const;
    void notifyInsertUpdates(const TimeLogEntry &data);
    void notifyInsertUpdates(const QVector<TimeLogEntry> &data);
    void notifyRemoveUpdates(const TimeLogEntry &data);
    void notifyEditUpdates(const TimeLogEntry &data, TimeLogHistory::Fields fields, QDateTime oldStart = QDateTime());
    void notifyUpdates(QSqlQuery &query,
                       TimeLogHistory::Fields fields = TimeLogHistory::DurationTime | TimeLogHistory::PrecedingStart) const;
    bool updateSize();
    bool updateCategories(const QDateTime &begin = QDateTime::fromTime_t(0),
                          const QDateTime &end = QDateTime::currentDateTime());
    QSharedPointer<TimeLogCategory> parseCategories(const QSet<QString> &categories) const;
    QByteArray calcHash(const QDateTime &begin, const QDateTime &end) const;
    void pushUndo(const Undo undo);
};

#endif // TIMELOGHISTORYWORKER_H
