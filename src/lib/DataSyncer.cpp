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

#include <QThread>

#include "DataSyncer.h"
#include "DataSyncerWorker.h"

DataSyncer::DataSyncer(TimeLogHistory *history, QObject *parent) :
    QObject(parent),
    m_isRunning(false),
    m_notifySync(true),
    m_notifyNextSync(false),
    m_autoSync(true),
    m_thread(new QThread(this)),
    m_worker(new DataSyncerWorker(history, this))
{
    connect(m_worker, SIGNAL(error(QString)), this, SLOT(syncError(QString)));
    connect(m_worker, SIGNAL(synced()), this, SLOT(syncFinished()));
    connect(m_worker, SIGNAL(started()), this, SLOT(syncStarted()));
    connect(m_worker, SIGNAL(stopped()), this, SLOT(syncStopped()));
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

void DataSyncer::pack(const QDateTime &start)
{
    QMetaObject::invokeMethod(m_worker, "pack", Qt::AutoConnection, Q_ARG(QDateTime, start));
}

bool DataSyncer::isRunning() const
{
    return m_isRunning;
}

void DataSyncer::setAutoSync(bool autoSync)
{
    if (m_autoSync == autoSync) {
        return;
    }

    m_autoSync = autoSync;

    QMetaObject::invokeMethod(m_worker, "setAutoSync", Qt::AutoConnection,
                              Q_ARG(bool, autoSync));

    emit autoSyncChanged(m_autoSync);
}

void DataSyncer::setSyncCacheSize(int syncCacheSize)
{
    if (m_syncCacheSize == syncCacheSize) {
        return;
    }

    m_syncCacheSize = syncCacheSize;

    QMetaObject::invokeMethod(m_worker, "setSyncCacheSize", Qt::AutoConnection,
                              Q_ARG(int, syncCacheSize));

    emit syncCacheSizeChanged(m_syncCacheSize);
}

void DataSyncer::setSyncCacheTimeout(int syncCacheTimeout)
{
    if (m_syncCacheTimeout == syncCacheTimeout) {
        return;
    }

    m_syncCacheTimeout = syncCacheTimeout;

    QMetaObject::invokeMethod(m_worker, "setSyncCacheTimeout", Qt::AutoConnection,
                              Q_ARG(int, syncCacheTimeout));

    emit syncCacheTimeoutChanged(m_syncCacheTimeout);
}

void DataSyncer::setSyncPath(const QUrl &syncPathUrl)
{
    if (m_syncPath == syncPathUrl) {
        return;
    }

    m_syncPath = syncPathUrl;

    QMetaObject::invokeMethod(m_worker, "setSyncPath", Qt::AutoConnection,
                              Q_ARG(QString, syncPathUrl.path()));

    emit syncPathChanged(m_syncPath);
}

void DataSyncer::setNoPack(bool noPack)
{
    QMetaObject::invokeMethod(m_worker, "setNoPack", Qt::AutoConnection, Q_ARG(bool, noPack));
}

void DataSyncer::sync(const QDateTime &start)
{
    QMetaObject::invokeMethod(m_worker, "sync", Qt::AutoConnection, Q_ARG(QDateTime, start));
}

void DataSyncer::syncError(const QString &errorText)
{
    if (m_notifyNextSync) {
        m_notifyNextSync = false;
    }

    emit error(errorText);
}

void DataSyncer::syncFinished()
{
    setIsRunning(false);

    if (!m_notifySync && !m_notifyNextSync) {
        return;
    } else if (m_notifyNextSync) {
        m_notifyNextSync = false;
    }

    emit synced(QPrivateSignal());
}

void DataSyncer::syncStarted()
{
    setIsRunning(true);
}

void DataSyncer::syncStopped()
{
    setIsRunning(false);
}

void DataSyncer::setIsRunning(bool isRunning)
{
    if (m_isRunning == isRunning) {
        return;
    }

    m_isRunning = isRunning;

    emit isRunningChanged(m_isRunning);
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
