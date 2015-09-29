#ifndef TIMELOGHISTORY_H
#define TIMELOGHISTORY_H

#include <QObject>
#include <QSet>

#include "TimeLogSyncData.h"

class QThread;

class TimeLogHistoryWorker;

class TimeLogHistory : public QObject
{
    Q_OBJECT
protected:
    explicit TimeLogHistory(QObject *parent = 0);
public:
    enum Fields {
        NoFields        = 0,
        StartTime       = 0x1,
        DurationTime    = 0x2,
        Category        = 0x4,
        Comment         = 0x8,
        AllFieldsMask   = StartTime | Category | Comment
    };
    Q_FLAG(Fields)

    virtual ~TimeLogHistory();

    static TimeLogHistory *instance();

    bool init();

    qlonglong size() const;
    QSet<QString> categories() const;

public slots:
    void insert(const TimeLogEntry &data);
    void insert(const QVector<TimeLogEntry> &data);
    void remove(const TimeLogEntry &data);
    void edit(const TimeLogEntry &data, TimeLogHistory::Fields fields);
    void sync(const QVector<TimeLogSyncData> &updatedData,
              const QVector<TimeLogSyncData> &removedData);

    void getHistoryBetween(const QDateTime &begin = QDateTime::fromTime_t(0),
                           const QDateTime &end = QDateTime::currentDateTime(),
                           const QString &category = QString()) const;
    void getHistoryAfter(const uint limit,
                         const QDateTime &from = QDateTime::fromTime_t(0)) const;
    void getHistoryBefore(const uint limit,
                          const QDateTime &until = QDateTime::currentDateTime()) const;

    void getSyncData(const QDateTime &mBegin = QDateTime::fromMSecsSinceEpoch(0),
                     const QDateTime &mEnd = QDateTime::currentDateTime()) const;

signals:
    void error(const QString &errorText) const;
    void dataAvailable(QDateTime from, QVector<TimeLogEntry> data) const;
    void dataAvailable(QVector<TimeLogEntry> data, QDateTime until) const;
    void dataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields>) const;
    void dataInserted(QVector<TimeLogEntry> data) const;
    void syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until) const;
    void dataSynced(QVector<TimeLogSyncData> updatedData, QVector<TimeLogSyncData> removedData);

private slots:
    void workerSizeChanged(qlonglong size);
    void workerCategoriesChanged(QSet<QString> categories);

private:
    void makeAsync();

    QThread *m_thread;
    TimeLogHistoryWorker *m_worker;

    qlonglong m_size;
    QSet<QString> m_categories;
};

#endif // TIMELOGHISTORY_H
