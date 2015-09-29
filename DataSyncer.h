#ifndef DATASYNCER_H
#define DATASYNCER_H

#include <QSet>

#include "AbstractDataInOut.h"
#include "TimeLogSyncData.h"

class DataSyncer : public AbstractDataInOut
{
    Q_OBJECT
public:
    explicit DataSyncer(QObject *parent = 0);

protected slots:
    virtual void startIO(const QString &path);
    virtual void historyError(const QString &errorText);

private slots:
    void syncDataAvailable(QVector<TimeLogSyncData> data, QDateTime until);
    void syncDataSynced(QVector<TimeLogSyncData> updatedData,
                        QVector<TimeLogSyncData> removedData);

private:
    void startSync(const QString &path);
    void startImport(const QString &path);
    void startExport(const QString &path);

    void syncFolders(const QString &path);
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
    QDir m_dir;
    QString m_syncPath;
    QSet<QString> m_outFiles;
    QSet<QString> m_inFiles;
};

#endif // DATASYNCER_H
