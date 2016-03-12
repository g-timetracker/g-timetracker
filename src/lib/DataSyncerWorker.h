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

#ifndef DATASYNCERWORKER_H
#define DATASYNCERWORKER_H

#include <QObject>
#include <QDir>
#include <QSet>

#include "TimeLogHistory.h"

class QStateMachine;
class QState;
class QFinalState;
class QTimer;
class QFileSystemWatcher;

class TimeLogHistory;
class DBSyncer;

class DataSyncerWorker : public QObject
{
    Q_OBJECT
public:
    explicit DataSyncerWorker(TimeLogHistory *db, QObject *parent = 0);

    Q_INVOKABLE void init(const QString &dataPath);

    Q_INVOKABLE void pack(const QDateTime &start = QDateTime::currentDateTimeUtc());

    Q_INVOKABLE void setAutoSync(bool autoSync);
    Q_INVOKABLE void setSyncCacheSize(int syncCacheSize);
    Q_INVOKABLE void setSyncCacheTimeout(int syncCacheTimeout);
    Q_INVOKABLE void setSyncPath(const QString &path);
    Q_INVOKABLE void setNoPack(bool noPack);

public slots:
    void sync(const QDateTime &start = QDateTime::currentDateTimeUtc());

signals:
    void error(const QString &errorText) const;

    void started(QPrivateSignal);
    void exported(QPrivateSignal);
    void foldersSynced(QPrivateSignal);
    void imported(QPrivateSignal);
    void dirsSynced(QPrivateSignal);
    void synced(QPrivateSignal);
    void stopped(QPrivateSignal);

private slots:
    void historyError(const QString &errorText);
    void historyDataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields);
    void historyDataInserted(const TimeLogEntry &data);
    void historyDataRemoved(const TimeLogEntry &data);
    void historyCategoriesChanged(QSharedPointer<TimeLogCategoryTreeNode> categories);
    void syncDataAvailable(QVector<TimeLogSyncDataEntry> entryData,
                           QVector<TimeLogSyncDataCategory> categoryData, QDateTime until);
    void syncExistsAvailable(bool isExists, QDateTime mBegin, QDateTime mEnd);
    void syncAmountAvailable(qlonglong size, QDateTime maxMTime, QDateTime mBegin, QDateTime mEnd);
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
    void syncDataSynced(const QDateTime &maxSyncDate);
    void syncFinished();

    void packImported(QDateTime latestMTime);
    void packExported(QDateTime latestMTime);

    void startImport();
    void startExport();
    void syncFolders();
    void packSync();
    void updateTimestamp();
    void cleanState();

    void checkSyncFolder();
    void syncWatcherEvent(const QString &path);

private:
    void compareWithDir(const QString &path);
    bool copyFiles(const QString &from, const QString &to, const QSet<QString> fileList,
                   bool isRemoveSource);
    bool copyFile(const QString &source, const QString &destination, bool isOverwrite,
                  bool isRemoveSource) const;
    bool exportFile(const QVector<TimeLogSyncDataEntry> &entryData,
                    const QVector<TimeLogSyncDataCategory> &categoryData);
    void importCurrentItem();
    void importFile(const QString &path);
    bool parseFile(const QString &path,
                   QVector<TimeLogSyncDataEntry> &updatedData,
                   QVector<TimeLogSyncDataEntry> &removedData,
                   QVector<TimeLogSyncDataCategory> &categoryData) const;
    void importPack(const QString &path);
    void processCurrentItemImported();
    void exportPack();
    QString formatSyncEntryChange(const TimeLogSyncDataEntry &oldData,
                                  const TimeLogSyncDataEntry &newData) const;
    QString formatSyncCategoryChange(const TimeLogSyncDataCategory &oldData,
                                     const TimeLogSyncDataCategory &newData) const;
    QDateTime maxPackPeriodStart() const;
    bool removeOldFiles(const QString &packName);
    void addCachedSyncChange();
    void addCachedSyncChanges(int count);
    void checkCachedSyncChanges();

    bool m_isInitialized;
    TimeLogHistory *m_db;
    QString m_internalSyncPath;
    QStateMachine *m_sm;
    QState *m_exportState;
    QState *m_syncFoldersState;
    QState *m_importState;
    QStateMachine *m_packSM;
    QState *m_packSMPackState;
    QFinalState *m_packSMFinalState;
    QState *m_timestampState;
    QFinalState *m_finalState;
    bool m_autoSync;
    int m_syncCacheSize;
    QString m_externalSyncPath;
    bool m_noPack;
    QTimer *m_syncStartTimer;
    QFileSystemWatcher *m_syncWatcher;
    QTimer *m_syncWatcherTimer;
    int m_syncCacheTimeout;
    QTimer *m_syncCacheTimer;

    QDir m_internalSyncDir;
    QString m_currentSyncPath;
    QDateTime m_syncStart;

    int m_cachedSyncChanges;
    QStringList m_fileList;
    int m_currentIndex;
    QSet<QString> m_outFiles;
    QSet<QString> m_inFiles;
    bool m_wroteToExternalSync;

    DBSyncer *m_dbSyncer;
    TimeLogHistory *m_pack;

    QString m_packName;
    QDateTime m_packMTime;
    bool m_forcePack;
};

#endif // DATASYNCERWORKER_H
