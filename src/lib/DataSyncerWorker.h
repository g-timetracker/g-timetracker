#ifndef DATASYNCERWORKER_H
#define DATASYNCERWORKER_H

#include <QObject>
#include <QDir>
#include <QSet>

#include "TimeLogSyncData.h"

class QStateMachine;
class QState;
class QFinalState;

class TimeLogHistory;
class DBSyncer;

class DataSyncerWorker : public QObject
{
    Q_OBJECT
public:
    explicit DataSyncerWorker(TimeLogHistory *db, QObject *parent = 0);

    void init(const QString &dataPath);

public slots:
    void sync(const QString &path, const QDateTime &start = QDateTime::currentDateTimeUtc());
    void setNoPack(bool noPack);
    void pack(const QString &path, const QDateTime &start = QDateTime::currentDateTimeUtc());

signals:
    void error(const QString &errorText) const;

    void started(QPrivateSignal);
    void exported(QPrivateSignal);
    void foldersSynced(QPrivateSignal);
    void imported(QPrivateSignal);
    void synced(QPrivateSignal);

private slots:
    void historyError(const QString &errorText);
    void syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until);
    void syncHasSyncData(bool hasData, QDateTime mBegin, QDateTime mEnd);
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
    void cleanState();

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

    bool m_isInitialized;
    TimeLogHistory *m_db;
    QString m_intPath;
    QStateMachine *m_sm;
    QState *m_exportState;
    QState *m_syncFoldersState;
    QState *m_importState;
    QState *m_packState;
    QFinalState *m_finalState;
    bool m_noPack;

    QDir m_dir;
    QString m_syncPath;
    QDateTime m_syncStart;

    QStringList m_fileList;
    int m_currentIndex;
    QSet<QString> m_outFiles;
    QSet<QString> m_inFiles;

    DBSyncer *m_dbSyncer;
    TimeLogHistory *m_pack;

    QString m_packName;
    QDateTime m_packMTime;
    bool m_forcePack;
};

#endif // DATASYNCERWORKER_H
