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

    void init(const QString &dataPath);

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
    void syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until);
    void syncDataAmountAvailable(qlonglong size, QDateTime maxMTime, QDateTime mBegin, QDateTime mEnd);
    void syncStatsAvailable(QVector<TimeLogSyncData> removedOld, QVector<TimeLogSyncData> removedNew,
                            QVector<TimeLogSyncData> insertedOld, QVector<TimeLogSyncData> insertedNew,
                            QVector<TimeLogSyncData> updatedOld, QVector<TimeLogSyncData> updatedNew) const;
    void syncDataSynced(QVector<TimeLogSyncData> updatedData,
                        QVector<TimeLogSyncData> removedData);
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
    bool exportFile(const QVector<TimeLogSyncData> &data);
    void importCurrentItem();
    void importFile(const QString &path);
    bool parseFile(const QString &path,
                   QVector<TimeLogSyncData> &updatedData,
                   QVector<TimeLogSyncData> &removedData) const;
    void importPack(const QString &path);
    void processCurrentItemImported();
    void exportPack();
    QString formatSyncChange(const TimeLogSyncData &oldData, const TimeLogSyncData &newData) const;
    QDateTime maxPackPeriodStart() const;
    bool removeOldFiles(const QString &packName);
    void addCachedSyncChanges(int count = 1);
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
