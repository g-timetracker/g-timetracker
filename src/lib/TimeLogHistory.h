#ifndef TIMELOGHISTORY_H
#define TIMELOGHISTORY_H

#include <QObject>
#include <QSet>

#include "TimeLogStats.h"
#include "TimeLogSyncData.h"

class QThread;

class TimeLogHistoryWorker;

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

    bool init(const QString &dataPath);

    qlonglong size() const;
    QSet<QString> categories() const;
    int undoCount() const;

public slots:
    void insert(const TimeLogEntry &data);
    void import(const QVector<TimeLogEntry> &data);
    void remove(const TimeLogEntry &data);
    void edit(const TimeLogEntry &data, TimeLogHistory::Fields fields);
    void editCategory(QString oldName, QString newName);
    void sync(const QVector<TimeLogSyncData> &updatedData,
              const QVector<TimeLogSyncData> &removedData);

    void undo();

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
    void dataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields>) const;
    void dataInserted(const TimeLogEntry &data) const;
    void dataImported(QVector<TimeLogEntry> data) const;
    void dataRemoved(const TimeLogEntry &data) const;
    void statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const;
    void syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until) const;
    void syncStatsAvailable(QVector<TimeLogSyncData> removedOld, QVector<TimeLogSyncData> removedNew,
                            QVector<TimeLogSyncData> insertedOld, QVector<TimeLogSyncData> insertedNew,
                            QVector<TimeLogSyncData> updatedOld, QVector<TimeLogSyncData> updatedNew) const;
    void dataSynced(QVector<TimeLogSyncData> updatedData, QVector<TimeLogSyncData> removedData);

    void categoriesChanged(QSet<QString> categories) const;
    void undoCountChanged(int undoCount) const;

private slots:
    void workerSizeChanged(qlonglong size);
    void workerCategoriesChanged(QSet<QString> categories);
    void workerUndoCountChanged(int undoCount);

private:
    void makeAsync();

    QThread *m_thread;
    TimeLogHistoryWorker *m_worker;

    qlonglong m_size;
    QSet<QString> m_categories;
    int m_undoCount;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TimeLogHistory::Fields)

#endif // TIMELOGHISTORY_H
