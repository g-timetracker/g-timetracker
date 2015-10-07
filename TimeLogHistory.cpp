#include <QThread>

#include "TimeLogHistory.h"
#include "TimeLogHistory_p.h"
#include "TimeLogHistoryWorker.h"

Q_GLOBAL_STATIC(TimeLogHistorySingleton, timeLogHistory)

TimeLogHistory::TimeLogHistory(QObject *parent) :
    QObject(parent),
    m_thread(new QThread(this)),
    m_worker(new TimeLogHistoryWorker(this)),
    m_size(0)
{
    connect(m_worker, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
    connect(m_worker, SIGNAL(dataAvailable(QDateTime,QVector<TimeLogEntry>)),
            this, SIGNAL(dataAvailable(QDateTime,QVector<TimeLogEntry>)));
    connect(m_worker, SIGNAL(dataAvailable(QVector<TimeLogEntry>,QDateTime)),
            this, SIGNAL(dataAvailable(QVector<TimeLogEntry>,QDateTime)));
    connect(m_worker, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)),
            this, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    connect(m_worker, SIGNAL(dataInserted(QVector<TimeLogEntry>)),
            this, SIGNAL(dataInserted(QVector<TimeLogEntry>)));
    connect(m_worker, SIGNAL(sizeChanged(qlonglong)),
            this, SLOT(workerSizeChanged(qlonglong)));
    connect(m_worker, SIGNAL(categoriesChanged(QSet<QString>)),
            this, SLOT(workerCategoriesChanged(QSet<QString>)));
    connect(m_worker, SIGNAL(statsDataAvailable(QVector<TimeLogStats>,QDateTime)),
            this, SIGNAL(statsDataAvailable(QVector<TimeLogStats>,QDateTime)));
    connect(m_worker, SIGNAL(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)),
            this, SIGNAL(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)));
    connect(m_worker, SIGNAL(dataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)),
            this, SIGNAL(dataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)));
}

TimeLogHistory::~TimeLogHistory()
{
    if (m_thread->isRunning()) {
        m_thread->quit();
    }
}

TimeLogHistory *TimeLogHistory::instance()
{
    return static_cast<TimeLogHistory*>(timeLogHistory);
}

bool TimeLogHistory::init(const QString &dataPath)
{
    if (m_worker->thread() != thread()) {
        return false;
    }

    if (!m_worker->init(dataPath)) {
        return false;
    }

    makeAsync();

    return true;
}

qlonglong TimeLogHistory::size() const
{
    return m_size;
}

QSet<QString> TimeLogHistory::categories() const
{
    return m_categories;
}

void TimeLogHistory::insert(const TimeLogEntry &data)
{
    QMetaObject::invokeMethod(m_worker, "insert", Qt::AutoConnection, Q_ARG(TimeLogEntry, data));
}

void TimeLogHistory::insert(const QVector<TimeLogEntry> &data)
{
    QMetaObject::invokeMethod(m_worker, "insert", Qt::AutoConnection,
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

void TimeLogHistory::sync(const QVector<TimeLogSyncData> &updatedData, const QVector<TimeLogSyncData> &removedData)
{
    QMetaObject::invokeMethod(m_worker, "sync", Qt::AutoConnection,
                              Q_ARG(QVector<TimeLogSyncData>, updatedData),
                              Q_ARG(QVector<TimeLogSyncData>, removedData));
}

void TimeLogHistory::getHistoryBetween(const QDateTime &begin, const QDateTime &end, const QString &category) const
{
    QMetaObject::invokeMethod(m_worker, "getHistoryBetween", Qt::AutoConnection, Q_ARG(QDateTime, begin),
                              Q_ARG(QDateTime, end), Q_ARG(QString, category));
}

void TimeLogHistory::getHistoryAfter(const uint limit, const QDateTime &from) const
{
    QMetaObject::invokeMethod(m_worker, "getHistoryAfter", Qt::AutoConnection, Q_ARG(uint, limit),
                              Q_ARG(QDateTime, from));
}

void TimeLogHistory::getHistoryBefore(const uint limit, const QDateTime &until) const
{
    QMetaObject::invokeMethod(m_worker, "getHistoryBefore", Qt::AutoConnection, Q_ARG(uint, limit),
                              Q_ARG(QDateTime, until));
}

void TimeLogHistory::getStats(const QDateTime &begin, const QDateTime &end, const QString &category) const
{
    QMetaObject::invokeMethod(m_worker, "getStats", Qt::AutoConnection, Q_ARG(QDateTime, begin),
                              Q_ARG(QDateTime, end), Q_ARG(QString, category));
}

void TimeLogHistory::getSyncData(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    QMetaObject::invokeMethod(m_worker, "getSyncData", Qt::AutoConnection,
                              Q_ARG(QDateTime, mBegin), Q_ARG(QDateTime, mEnd));
}

void TimeLogHistory::workerSizeChanged(qlonglong size)
{
    m_size = size;
}

void TimeLogHistory::workerCategoriesChanged(QSet<QString> categories)
{
    m_categories.swap(categories);
}

void TimeLogHistory::makeAsync()
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
