#ifndef DATASYNCER_H
#define DATASYNCER_H

#include <QSet>

#include "AbstractDataInOut.h"
#include "TimeLogSyncData.h"

class QStateMachine;

class DataSyncer : public AbstractDataInOut
{
    Q_OBJECT
protected:
    explicit DataSyncer(QObject *parent = 0);

public:
    static DataSyncer *instance();
    virtual ~DataSyncer();

signals:
    void error(const QString &errorText) const;

    void started(QPrivateSignal);
    void exported(QPrivateSignal);
    void foldersSynced(QPrivateSignal);
    void synced(QPrivateSignal);

protected slots:
    virtual void startIO(const QString &path);
    virtual void historyError(const QString &errorText);

private slots:
    void syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until);
    void syncDataSynced(QVector<TimeLogSyncData> updatedData,
                        QVector<TimeLogSyncData> removedData);

    void startImport();
    void startExport();
    void syncFolders();

private:
    void compareWithDir(const QString &path);
    bool copyFiles(const QString &from, const QString &to, const QSet<QString> fileList,
                   bool isRemoveSource);
    bool copyFile(const QString &source, const QString &destination, bool isOverwrite,
                  bool isRemoveSource) const;
    bool exportData(const QVector<TimeLogSyncData> &data);
    void importCurrentFile();
    void importFile(const QString &path);
    bool parseFile(const QString &path,
                   QVector<TimeLogSyncData> &updatedData,
                   QVector<TimeLogSyncData> &removedData) const;

    QStringList m_fileList;
    int m_currentIndex;
    QString m_intPath;
    QDir m_dir;
    QString m_syncPath;
    QSet<QString> m_outFiles;
    QSet<QString> m_inFiles;
    QStateMachine *m_sm;
};

#endif // DATASYNCER_H
