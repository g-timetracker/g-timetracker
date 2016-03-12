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

#include "TimeLogHistory.h"
#include "TimeLogHistoryWorker.h"

TimeLogHistory::TimeLogHistory(QObject *parent) :
    QObject(parent),
    m_thread(new QThread()),
    m_worker(new TimeLogHistoryWorker()),
    m_size(0),
    m_undoCount(0)
{
    connect(m_worker, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
    connect(m_worker, SIGNAL(dataOutdated()),
            this, SIGNAL(dataOutdated()));
    connect(m_worker, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)),
            this, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));
    connect(m_worker, SIGNAL(storedCategoriesAvailable(QVector<TimeLogCategory>)),
            this, SIGNAL(storedCategoriesAvailable(QVector<TimeLogCategory>)));
    connect(m_worker, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)),
            this, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    connect(m_worker, SIGNAL(dataInserted(TimeLogEntry)),
            this, SIGNAL(dataInserted(TimeLogEntry)));
    connect(m_worker, SIGNAL(dataImported(QVector<TimeLogEntry>)),
            this, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    connect(m_worker, SIGNAL(dataRemoved(TimeLogEntry)),
            this, SIGNAL(dataRemoved(TimeLogEntry)));
    connect(m_worker, SIGNAL(sizeChanged(qlonglong)),
            this, SLOT(workerSizeChanged(qlonglong)));
    connect(m_worker, SIGNAL(undoCountChanged(int)),
            this, SLOT(workerUndoCountChanged(int)));
    connect(m_worker, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)),
            this, SLOT(workerCategoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));
    connect(m_worker, SIGNAL(statsDataAvailable(QVector<TimeLogStats>,QDateTime)),
            this, SIGNAL(statsDataAvailable(QVector<TimeLogStats>,QDateTime)));
    connect(m_worker, SIGNAL(syncDataAvailable(QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataCategory>,QDateTime)),
            this, SIGNAL(syncDataAvailable(QVector<TimeLogSyncDataEntry>,
                                           QVector<TimeLogSyncDataCategory>,QDateTime)));
    connect(m_worker, SIGNAL(syncAmountAvailable(qlonglong,QDateTime,QDateTime,QDateTime)),
            this, SIGNAL(syncAmountAvailable(qlonglong,QDateTime,QDateTime,QDateTime)));
    connect(m_worker, SIGNAL(syncExistsAvailable(bool,QDateTime,QDateTime)),
            this, SIGNAL(syncExistsAvailable(bool,QDateTime,QDateTime)));
    connect(m_worker, SIGNAL(syncEntryStatsAvailable(QVector<TimeLogSyncDataEntry>,
                                                     QVector<TimeLogSyncDataEntry>,
                                                     QVector<TimeLogSyncDataEntry>,
                                                     QVector<TimeLogSyncDataEntry>,
                                                     QVector<TimeLogSyncDataEntry>,
                                                     QVector<TimeLogSyncDataEntry>)),
            this, SIGNAL(syncEntryStatsAvailable(QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>,
                                                 QVector<TimeLogSyncDataEntry>)));
    connect(m_worker, SIGNAL(syncCategoryStatsAvailable(QVector<TimeLogSyncDataCategory>,
                                                        QVector<TimeLogSyncDataCategory>,
                                                        QVector<TimeLogSyncDataCategory>,
                                                        QVector<TimeLogSyncDataCategory>,
                                                        QVector<TimeLogSyncDataCategory>,
                                                        QVector<TimeLogSyncDataCategory>)),
            this, SIGNAL(syncCategoryStatsAvailable(QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>,
                                                    QVector<TimeLogSyncDataCategory>)));
    connect(m_worker, SIGNAL(hashesAvailable(QMap<QDateTime,QByteArray>)),
            this, SIGNAL(hashesAvailable(QMap<QDateTime,QByteArray>)));
    connect(m_worker, SIGNAL(dataSynced(QDateTime)),
            this, SIGNAL(dataSynced(QDateTime)));
    connect(m_worker, SIGNAL(hashesUpdated()),
            this, SIGNAL(hashesUpdated()));

    connect(m_thread, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
    connect(m_worker, SIGNAL(destroyed()), m_thread, SLOT(deleteLater()));

    m_worker->moveToThread(m_thread);
    m_thread->start();
}

TimeLogHistory::~TimeLogHistory()
{
    if (m_thread->isRunning()) {
        m_thread->quit();
    }
}

bool TimeLogHistory::init(const QString &dataPath, const QString &filePath, bool isReadonly, bool isPopulateCategories)
{
    bool result = false;

    QMetaObject::invokeMethod(m_worker, "init", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(bool, result), Q_ARG(QString, dataPath),
                              Q_ARG(QString, filePath), Q_ARG(bool, isReadonly),
                              Q_ARG(bool, isPopulateCategories));

    return result;
}

qlonglong TimeLogHistory::size() const
{
    return m_size;
}

QSharedPointer<TimeLogCategoryTreeNode> TimeLogHistory::categories() const
{
    return m_categories;
}

int TimeLogHistory::undoCount() const
{
    return m_undoCount;
}

void TimeLogHistory::insert(const TimeLogEntry &data)
{
    QMetaObject::invokeMethod(m_worker, "insert", Qt::AutoConnection, Q_ARG(TimeLogEntry, data));
}

void TimeLogHistory::import(const QVector<TimeLogEntry> &data)
{
    QMetaObject::invokeMethod(m_worker, "import", Qt::AutoConnection,
                              Q_ARG(QVector<TimeLogEntry>, data));
}

void TimeLogHistory::remove(const TimeLogEntry &data)
{
    QMetaObject::invokeMethod(m_worker, "remove", Qt::AutoConnection, Q_ARG(TimeLogEntry, data));
}

void TimeLogHistory::edit(const TimeLogEntry &data, TimeLogHistory::Fields fields)
{
    QMetaObject::invokeMethod(m_worker, "edit", Qt::AutoConnection, Q_ARG(TimeLogEntry, data),
                              Q_ARG(TimeLogHistory::Fields, fields));
}

void TimeLogHistory::addCategory(const TimeLogCategory &category)
{
    QMetaObject::invokeMethod(m_worker, "addCategory", Qt::AutoConnection,
                              Q_ARG(TimeLogCategory, category));
}

void TimeLogHistory::removeCategory(const QString &name)
{
    QMetaObject::invokeMethod(m_worker, "removeCategory", Qt::AutoConnection,
                              Q_ARG(QString, name));
}

void TimeLogHistory::editCategory(const QString &oldName, const TimeLogCategory &category)
{
    QMetaObject::invokeMethod(m_worker, "editCategory", Qt::AutoConnection,
                              Q_ARG(QString, oldName), Q_ARG(TimeLogCategory, category));
}

void TimeLogHistory::sync(const QVector<TimeLogSyncDataEntry> &updatedData,
                          const QVector<TimeLogSyncDataEntry> &removedData,
                          const QVector<TimeLogSyncDataCategory> &categoryData)
{
    QMetaObject::invokeMethod(m_worker, "sync", Qt::AutoConnection,
                              Q_ARG(QVector<TimeLogSyncDataEntry>, updatedData),
                              Q_ARG(QVector<TimeLogSyncDataEntry>, removedData),
                              Q_ARG(QVector<TimeLogSyncDataCategory>, categoryData));
}

void TimeLogHistory::updateHashes()
{
    QMetaObject::invokeMethod(m_worker, "updateHashes", Qt::AutoConnection);
}

void TimeLogHistory::undo()
{
    QMetaObject::invokeMethod(m_worker, "undo", Qt::AutoConnection);
}

void TimeLogHistory::getHistoryBetween(qlonglong id, const QDateTime &begin, const QDateTime &end,
                                       const QString &category, bool withSubcategories) const
{
    QMetaObject::invokeMethod(m_worker, "getHistoryBetween", Qt::AutoConnection, Q_ARG(qlonglong, id),
                              Q_ARG(QDateTime, begin), Q_ARG(QDateTime, end), Q_ARG(QString, category),
                              Q_ARG(bool, withSubcategories));
}

void TimeLogHistory::getHistoryAfter(qlonglong id, const uint limit, const QDateTime &from) const
{
    QMetaObject::invokeMethod(m_worker, "getHistoryAfter", Qt::AutoConnection, Q_ARG(qlonglong, id),
                              Q_ARG(uint, limit), Q_ARG(QDateTime, from));
}

void TimeLogHistory::getHistoryBefore(qlonglong id, const uint limit, const QDateTime &until) const
{
    QMetaObject::invokeMethod(m_worker, "getHistoryBefore", Qt::AutoConnection, Q_ARG(qlonglong, id),
                              Q_ARG(uint, limit), Q_ARG(QDateTime, until));
}

void TimeLogHistory::getStoredCategories() const
{
    QMetaObject::invokeMethod(m_worker, "getStoredCategories", Qt::AutoConnection);
}

void TimeLogHistory::getStats(const QDateTime &begin, const QDateTime &end, const QString &category, const QString &separator) const
{
    QMetaObject::invokeMethod(m_worker, "getStats", Qt::AutoConnection,
                              Q_ARG(QDateTime, begin), Q_ARG(QDateTime, end),
                              Q_ARG(QString, category), Q_ARG(QString, separator));
}

void TimeLogHistory::getSyncData(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    QMetaObject::invokeMethod(m_worker, "getSyncData", Qt::AutoConnection,
                              Q_ARG(QDateTime, mBegin), Q_ARG(QDateTime, mEnd));
}

void TimeLogHistory::getSyncExists(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    QMetaObject::invokeMethod(m_worker, "getSyncExists", Qt::AutoConnection,
                              Q_ARG(QDateTime, mBegin), Q_ARG(QDateTime, mEnd));
}

void TimeLogHistory::getSyncAmount(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    QMetaObject::invokeMethod(m_worker, "getSyncAmount", Qt::AutoConnection,
                              Q_ARG(QDateTime, mBegin), Q_ARG(QDateTime, mEnd));
}

void TimeLogHistory::getHashes(const QDateTime &maxDate, bool noUpdate)
{
    QMetaObject::invokeMethod(m_worker, "getHashes", Qt::AutoConnection,
                              Q_ARG(QDateTime, maxDate), Q_ARG(bool, noUpdate));
}

void TimeLogHistory::workerSizeChanged(qlonglong size)
{
    if (m_size == size) {
        return;
    }

    m_size = size;

    emit sizeChanged(m_size);
}

void TimeLogHistory::workerCategoriesChanged(QSharedPointer<TimeLogCategoryTreeNode> categories)
{
    m_categories = categories;

    emit categoriesChanged(m_categories);
}

void TimeLogHistory::workerUndoCountChanged(int undoCount)
{
    if (m_undoCount == undoCount) {
        return;
    }

    m_undoCount = undoCount;

    emit undoCountChanged(m_undoCount);
}
