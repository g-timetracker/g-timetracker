#ifndef TIMELOGHISTORY_H
#define TIMELOGHISTORY_H

#include <QObject>
#include <QSharedPointer>

#include "TimeLogStats.h"
#include "TimeLogSyncDataEntry.h"
#include "TimeLogSyncDataCategory.h"

class QThread;

class TimeLogHistoryWorker;
class TimeLogCategoryTreeNode;

class TimeLogHistory : public QObject
{
    Q_OBJECT
public:
    enum Field {
        NoFields        = 0,
        StartTime       = 0x01,
        DurationTime    = 0x02,
        Category        = 0x04,
        Comment         = 0x08,
        PrecedingStart  = 0x10,
        AllFieldsMask   = StartTime | Category | Comment
    };
    Q_DECLARE_FLAGS(Fields, Field)
    Q_FLAG(Fields)

    explicit TimeLogHistory(QObject *parent = 0);
    virtual ~TimeLogHistory();

    bool init(const QString &dataPath, const QString &filePath = QString(),
              bool isPopulateCategories = false);

    qlonglong size() const;
    QSharedPointer<TimeLogCategoryTreeNode> categories() const;
    int undoCount() const;

public slots:
    void insert(const TimeLogEntry &data);
    void import(const QVector<TimeLogEntry> &data);
    void remove(const TimeLogEntry &data);
    void edit(const TimeLogEntry &data, TimeLogHistory::Fields fields);
    void addCategory(const TimeLogCategory &category);
    void removeCategory(const QString &name);
    void editCategory(const QString &oldName, const TimeLogCategory &category);
    void sync(const QVector<TimeLogSyncDataEntry> &updatedData,
              const QVector<TimeLogSyncDataEntry> &removedData,
              const QVector<TimeLogSyncDataCategory> &categoryData);
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

    void getStoredCategories() const;

    void getStats(const QDateTime &begin = QDateTime::fromTime_t(0, Qt::UTC),
                  const QDateTime &end = QDateTime::currentDateTimeUtc(),
                  const QString &category = QString(),
                  const QString &separator = ">") const;

    void getSyncData(const QDateTime &mBegin = QDateTime(),
                     const QDateTime &mEnd = QDateTime()) const;
    void getSyncExists(const QDateTime &mBegin = QDateTime::fromMSecsSinceEpoch(0, Qt::UTC),
                       const QDateTime &mEnd = QDateTime::currentDateTimeUtc()) const;
    void getSyncAmount(const QDateTime &mBegin = QDateTime::fromMSecsSinceEpoch(0, Qt::UTC),
                           const QDateTime &mEnd = QDateTime::currentDateTimeUtc()) const;

    void getHashes(const QDateTime &maxDate = QDateTime(), bool noUpdate = false);

signals:
    void error(const QString &errorText) const;
    void dataOutdated() const;
    void historyRequestCompleted(QVector<TimeLogEntry> data, qlonglong id) const;
    void storedCategoriesAvailable(QVector<TimeLogCategory> data) const;
    void dataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields>) const;
    void dataInserted(const TimeLogEntry &data) const;
    void dataImported(QVector<TimeLogEntry> data) const;
    void dataRemoved(const TimeLogEntry &data) const;
    void statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const;
    void syncDataAvailable(QVector<TimeLogSyncDataEntry> entryData,
                           QVector<TimeLogSyncDataCategory> categoryData, QDateTime until) const;
    void syncAmountAvailable(qlonglong size, QDateTime maxMTime, QDateTime mBegin, QDateTime mEnd) const;
    void syncExistsAvailable(bool isExists, QDateTime mBegin, QDateTime mEnd) const;
    void syncEntryStatsAvailable(QVector<TimeLogSyncDataEntry> removedOld,
                                 QVector<TimeLogSyncDataEntry> removedNew,
                                 QVector<TimeLogSyncDataEntry> insertedOld,
                                 QVector<TimeLogSyncDataEntry> insertedNew,
                                 QVector<TimeLogSyncDataEntry> updatedOld,
                                 QVector<TimeLogSyncDataEntry> updatedNew) const;
    void syncCategoryStatsAvailable(QVector<TimeLogSyncDataCategory> removedOld,
                                    QVector<TimeLogSyncDataCategory> removedNew,
                                    QVector<TimeLogSyncDataCategory> addedOld,
                                    QVector<TimeLogSyncDataCategory> addedNew,
                                    QVector<TimeLogSyncDataCategory> updatedOld,
                                    QVector<TimeLogSyncDataCategory> updatedNew) const;
    void hashesAvailable(QMap<QDateTime, QByteArray> hashes) const;
    void dataSynced(QVector<TimeLogSyncDataEntry> updatedData,
                    QVector<TimeLogSyncDataEntry> removedData);
    void hashesUpdated() const;

    void categoriesChanged(const QSharedPointer<TimeLogCategoryTreeNode> &categories) const;
    void undoCountChanged(int undoCount) const;

private slots:
    void workerSizeChanged(qlonglong size);
    void workerCategoriesChanged(QSharedPointer<TimeLogCategoryTreeNode> categories);
    void workerUndoCountChanged(int undoCount);

private:
    void makeAsync();

    QThread *m_thread;
    TimeLogHistoryWorker *m_worker;

    qlonglong m_size;
    QSharedPointer<TimeLogCategoryTreeNode> m_categories;
    int m_undoCount;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TimeLogHistory::Fields)

#endif // TIMELOGHISTORY_H
