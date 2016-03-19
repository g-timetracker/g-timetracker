#ifndef DATASYNCER_H
#define DATASYNCER_H

#include <QObject>
#include <QUrl>
#include <QDateTime>

class QThread;

class TimeLogHistory;
class DataSyncerWorker;

class DataSyncer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(bool autoSync MEMBER m_autoSync WRITE setAutoSync NOTIFY autoSyncChanged)
    Q_PROPERTY(int syncCacheSize MEMBER m_syncCacheSize WRITE setSyncCacheSize NOTIFY syncCacheSizeChanged)
    Q_PROPERTY(QUrl syncPath MEMBER m_syncPath WRITE setSyncPath NOTIFY syncPathChanged)
public:
    explicit DataSyncer(TimeLogHistory *history, QObject *parent = 0);
    virtual ~DataSyncer();

    void init(const QString &dataPath);
    void pack(const QDateTime &start = QDateTime::currentDateTimeUtc());

    bool isRunning() const;

    void setAutoSync(bool autoSync);
    void setSyncCacheSize(int syncCacheSize);
    void setSyncPath(const QUrl &syncPathUrl);
    void setNoPack(bool noPack);

signals:
    void isRunningChanged(bool newIsRunning) const;
    void autoSyncChanged(bool newAutoSync) const;
    void syncCacheSizeChanged(int newSyncCacheSize) const;
    void syncPathChanged(const QUrl &newSyncPath) const;
    void error(const QString &errorText) const;
    void synced(QPrivateSignal);

public slots:
    void sync(const QDateTime &start = QDateTime::currentDateTimeUtc());

private slots:
    void syncStarted();
    void syncStopped();

private:
    void makeAsync();
    void setIsRunning(bool isRunning);

    bool m_isRunning;
    bool m_autoSync;
    int m_syncCacheSize;
    QUrl m_syncPath;
    QThread *m_thread;
    DataSyncerWorker *m_worker;
};

#endif // DATASYNCER_H
