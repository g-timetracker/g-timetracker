#include <QThread>

#include "DataSyncer.h"
#include "DataSyncer_p.h"
#include "DataSyncerWorker.h"

Q_GLOBAL_STATIC(DataSyncerSingleton, dataSyncer)

DataSyncer::DataSyncer(QObject *parent) :
    QObject(parent),
    m_thread(new QThread),
    m_worker(new DataSyncerWorker)
{
    connect(m_worker, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
    connect(m_worker, SIGNAL(synced()), this, SIGNAL(synced()));

    connect(m_thread, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
    connect(m_worker, SIGNAL(destroyed()), m_thread, SLOT(deleteLater()));

    m_worker->moveToThread(m_thread);

    m_thread->start();
}

DataSyncer::~DataSyncer()
{
    m_thread->quit();
}

DataSyncer *DataSyncer::instance()
{
    return static_cast<DataSyncer*>(dataSyncer);
}

void DataSyncer::sync(const QUrl &pathUrl)
{
    QMetaObject::invokeMethod(m_worker, "start", Qt::AutoConnection, Q_ARG(QString, pathUrl.path()));
}
