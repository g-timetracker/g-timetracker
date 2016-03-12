/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef TIMELOGHISTORYWORKER_H
#define TIMELOGHISTORYWORKER_H

#include <QObject>
#include <QSqlQuery>
#include <QSet>
#include <QStack>
#include <QSharedPointer>
#include <QRegularExpression>

#include "TimeLogHistory.h"

class TimeLogCategoryTreeNode;

class TimeLogHistoryWorker : public QObject
{
    Q_OBJECT
public:
    explicit TimeLogHistoryWorker(QObject *parent = 0);
    ~TimeLogHistoryWorker();

    Q_INVOKABLE bool init(const QString &dataPath, const QString &filePath = QString(),
                          bool isReadonly = false, bool isPopulateCategories = false);
    qlonglong size() const;
    QSharedPointer<TimeLogCategoryTreeNode> categories() const;

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
                           const QString &category = QString(),
                           bool withSubcategories = false) const;
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
    void dataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields) const;
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
    void dataSynced(QDateTime maxSyncDate) const;
    void hashesUpdated() const;

    void sizeChanged(qlonglong size) const;
    void categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode> categories) const;
    void undoCountChanged(int undoCount) const;

private:
    class Undo
    {
    public:
        enum Type {
            InsertEntry,
            RemoveEntry,
            EditEntry,
            AddCategory,
            RemoveCategory,
            EditCategory,
            MergeCategories
        };

        Type type;
        QVector<TimeLogEntry> entryData;
        QVector<TimeLogHistory::Fields> entryFields;
        TimeLogCategory categoryData;
        QString categoryNewName;
    };

    bool m_isInitialized;
    QString m_connectionName;
    qlonglong m_size;
    QMap<QString, TimeLogCategory> m_categories;
    QHash<QString, int> m_categoryRecordsCount;
    QSharedPointer<TimeLogCategoryTreeNode> m_categoryTree;
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

    bool prepareAndExecQuery(QSqlQuery &query, const QString &queryString) const;
    qlonglong getSchemaVersion() const;
    bool setSchemaVersion(qint32 schemaVersion);
    bool setupTable();
    bool setupTriggers();
    void setSize(qlonglong size);
    void decrementCategoryCount(const QString &name);
    void incrementCategoryCount(const QString &name);
    void processFail();

    void insertEntry(const TimeLogEntry &data);
    void removeEntry(const TimeLogEntry &data);
    bool editEntry(const TimeLogEntry &data, TimeLogHistory::Fields fields);
    bool editEntries(const QVector<TimeLogEntry> &data, const QVector<TimeLogHistory::Fields> &fields);
    bool syncEntries(const QVector<TimeLogSyncDataEntry> &updatedData,
                     const QVector<TimeLogSyncDataEntry> &removedData, QDateTime &maxSyncDate);
    bool syncCategories(const QVector<TimeLogSyncDataCategory> &categoryData, QDateTime &maxSyncDate);

    bool insertEntryData(const QVector<TimeLogEntry> &data);
    bool insertEntryData(const TimeLogSyncDataEntry &data);
    bool removeEntryData(const TimeLogSyncDataEntry &data);
    bool editEntryData(const TimeLogSyncDataEntry &data, TimeLogHistory::Fields fields);
    bool editEntriesCategory(const QString &oldName, const QString &newName);
    bool addCategoryData(const TimeLogSyncDataCategory &data);
    bool removeCategoryData(const TimeLogSyncDataCategory &data);
    bool editCategoryData(const QString &oldName, const TimeLogSyncDataCategory &data);
    bool syncEntryData(const QVector<TimeLogSyncDataEntry> &removed,
                       const QVector<TimeLogSyncDataEntry> &inserted,
                       const QVector<TimeLogSyncDataEntry> &updated,
                       const QVector<TimeLogHistory::Fields> &updateFields);
    bool syncCategoryData(const QVector<TimeLogSyncDataCategory> &removed,
                          const QVector<TimeLogSyncDataCategory> &inserted,
                          const QVector<TimeLogSyncDataCategory> &updatedNew,
                          const QVector<TimeLogSyncDataCategory> &updatedOld);
    void updateDataHashes(const QMap<QDateTime, QByteArray> &hashes);
    bool writeHash(const QDateTime &start, const QByteArray &hash);
    bool removeHash(const QDateTime &start);
    QVector<TimeLogEntry> getHistory(QSqlQuery &query) const;
    QVector<TimeLogStats> getStats(QSqlQuery &query) const;
    QVector<TimeLogSyncDataEntry> getSyncEntryData(const QDateTime &mBegin = QDateTime(),
                                              const QDateTime &mEnd = QDateTime()) const;
    QVector<TimeLogSyncDataEntry> getSyncEntryData(QSqlQuery &query) const;
    QVector<TimeLogSyncDataCategory> getSyncCategoryData(const QDateTime &mBegin = QDateTime(),
                                                         const QDateTime &mEnd = QDateTime()) const;
    QVector<TimeLogSyncDataCategory> getSyncCategoryData(QSqlQuery &query) const;
    TimeLogEntry getEntry(const QUuid &uuid);
    QVector<TimeLogEntry> getEntries(const QString &category) const;
    QVector<TimeLogSyncDataEntry> getSyncEntriesAffected(const QUuid &uuid);
    QVector<TimeLogSyncDataCategory> getSyncCategoriesAffected(const QUuid &uuid);
    bool getSyncDataExists(const QDateTime &mBegin = QDateTime::fromMSecsSinceEpoch(0, Qt::UTC),
                           const QDateTime &mEnd = QDateTime::currentDateTimeUtc()) const;
    QMap<QDateTime, QByteArray> getDataHashes(const QDateTime &maxDate = QDateTime()) const;
    void notifyInsertUpdates(const TimeLogEntry &data);
    void notifyInsertUpdates(const QVector<TimeLogEntry> &data);
    void notifyRemoveUpdates(const TimeLogEntry &data);
    void notifyEditUpdates(const TimeLogEntry &data, TimeLogHistory::Fields fields, QDateTime oldStart = QDateTime());
    void notifyUpdates(QSqlQuery &query,
                       TimeLogHistory::Fields fields = TimeLogHistory::DurationTime | TimeLogHistory::PrecedingStart) const;
    bool startTransaction(QSqlDatabase &db);
    bool commitTransaction(QSqlDatabase &db);
    void rollbackTransaction(QSqlDatabase &db);
    QString fixCategoryName(const QString &name) const;
    bool fetchCategories();
    bool checkIsDBEmpty();
    bool populateCategories();
    void updateCategories();
    QSharedPointer<TimeLogCategoryTreeNode> parseCategories(const QStringList &categories) const;
    QByteArray calcHash(const QDateTime &begin, const QDateTime &end) const;
    void pushUndo(const Undo undo);
};

#endif // TIMELOGHISTORYWORKER_H
