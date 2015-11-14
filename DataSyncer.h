#ifndef DATASYNCER_H
#define DATASYNCER_H

#include <QObject>
#include <QUrl>

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
    void sync(const QUrl &pathUrl);

private:
    void makeAsync();

    QThread *m_thread;
    DataSyncerWorker *m_worker;
};

#endif // DATASYNCER_H
