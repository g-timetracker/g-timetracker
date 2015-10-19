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
    void insert(const QVector<TimeLogEntry> &data);
    void remove(const TimeLogEntry &data);
    void edit(const TimeLogEntry &data, TimeLogHistory::Fields fields);
    void editCategory(QString oldName, QString newName);
    void sync(const QVector<TimeLogSyncData> &updatedData,
              const QVector<TimeLogSyncData> &removedData);

    void getHistoryBetween(const QDateTime &begin = QDateTime::fromTime_t(0),
                           const QDateTime &end = QDateTime::currentDateTime(),
                           const QString &category = QString()) const;
    void getHistoryAfter(const uint limit,
                         const QDateTime &from = QDateTime::fromTime_t(0)) const;
    void getHistoryBefore(const uint limit,
                          const QDateTime &until = QDateTime::currentDateTime()) const;

    void getStats(const QDateTime &begin = QDateTime::fromTime_t(0),
                  const QDateTime &end = QDateTime::currentDateTime(),
                  const QString &category = QString()) const;

    void getSyncData(const QDateTime &mBegin = QDateTime::fromMSecsSinceEpoch(0),
                     const QDateTime &mEnd = QDateTime::currentDateTime()) const;

signals:
    void error(const QString &errorText) const;
    void dataChanged() const;
    void dataAvailable(QDateTime from, QVector<TimeLogEntry> data) const;
    void dataAvailable(QVector<TimeLogEntry> data, QDateTime until) const;
    void dataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields) const;
    void dataInserted(QVector<TimeLogEntry> data) const;
    void statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const;
    void syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until) const;
    void dataSynced(QVector<TimeLogSyncData> updatedData, QVector<TimeLogSyncData> removedData);

    void sizeChanged(qlonglong size) const;
    void categoriesChanged(QSet<QString> categories) const;

private:
    bool m_isInitialized;
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
    bool syncData(const QVector<TimeLogSyncData> &updatedData,
                  const QVector<TimeLogSyncData> &removedData);
    QVector<TimeLogEntry> getHistory(QSqlQuery &query) const;
    QVector<TimeLogStats> getStats(QSqlQuery &query) const;
    QVector<TimeLogSyncData> getSyncData(QSqlQuery &query) const;
    bool notifyUpdates(const QString &queryString, const QVector<QDateTime> &values) const;
    bool updateSize();
    bool updateCategories(const QDateTime &begin = QDateTime::fromTime_t(0),
                          const QDateTime &end = QDateTime::currentDateTime());
};

#endif // TIMELOGHISTORYWORKER_H
