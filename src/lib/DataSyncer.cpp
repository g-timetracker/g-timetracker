#include <QThread>

#include "DataSyncer.h"
#include "DataSyncerWorker.h"

DataSyncer::DataSyncer(TimeLogHistory *history, QObject *parent) :
    QObject(parent),
    m_thread(new QThread(this)),
    m_worker(new DataSyncerWorker(history, this))
{
    connect(m_worker, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
    connect(m_worker, SIGNAL(synced()), this, SIGNAL(synced()));
}

DataSyncer::~DataSyncer()
{
    if (m_thread->isRunning()) {
        m_thread->quit();
    }
}

void DataSyncer::init(const QString &dataPath)
{
    if (m_worker->thread() != thread()) {
        return;
    }

    m_worker->init(dataPath);

    makeAsync();
}

void DataSyncer::sync(const QUrl &pathUrl, const QDateTime &start)
{
    QMetaObject::invokeMethod(m_worker, "sync", Qt::AutoConnection, Q_ARG(QString, pathUrl.path()),
                              Q_ARG(QDateTime, start));
}

void DataSyncer::setNoPack(bool noPack)
{
    QMetaObject::invokeMethod(m_worker, "setNoPack", Qt::AutoConnection, Q_ARG(bool, noPack));
}

void DataSyncer::pack(const QString &path, const QDateTime &start)
{
    QMetaObject::invokeMethod(m_worker, "pack", Qt::AutoConnection, Q_ARG(QString, path),
                              Q_ARG(QDateTime, start));
}

void DataSyncer::makeAsync()
{
    if (m_worker->thread() != thread()) {
        return;
    }

    m_thread->setParent(0);
    m_worker->setParent(0);

    connect(m_thread, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
    connect(m_worker, SIGNAL(destroyed()), m_thread, SLOT(deleteLater()));

    m_worker->moveToThread(m_thread);

    m_thread->start();
}
