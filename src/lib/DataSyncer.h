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
public:
    explicit DataSyncer(TimeLogHistory *history, QObject *parent = 0);
    virtual ~DataSyncer();

    void init(const QString &dataPath);

signals:
    void error(const QString &errorText) const;
    void synced(QPrivateSignal);

public slots:
    void sync(const QUrl &pathUrl, const QDateTime &start = QDateTime::currentDateTimeUtc());
    void setNoPack(bool noPack);
    void pack(const QString &path, const QDateTime &start = QDateTime::currentDateTimeUtc());

private:
    void makeAsync();

    QThread *m_thread;
    DataSyncerWorker *m_worker;
};

#endif // DATASYNCER_H
