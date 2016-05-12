#include <QStateMachine>
#include <QFinalState>

#include "DBSyncer.h"
#include "TimeLogHistory.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(DB_SYNCER_CATEGORY, "DBSyncer", QtInfoMsg)

DBSyncer::DBSyncer(TimeLogHistory *source, TimeLogHistory *destination, QObject *parent) :
    QObject(parent),
    m_source(source),
    m_destination(destination),
    m_sm(new QStateMachine(this)),
    m_sourceHashesState(new QState()),
    m_destinationHashesState(new QState()),
    m_syncState(new QState()),
    m_updateHashesState(new QState()),
    m_finalState(new QFinalState()),
    m_isRecalcHashes(false)
{
    m_sourceHashesState->addTransition(this, SIGNAL(sourceHashesChecked()), m_destinationHashesState);
    m_destinationHashesState->addTransition(this, SIGNAL(destinationHashesChecked()), m_syncState);
    m_syncState->addTransition(this, SIGNAL(synced()), m_updateHashesState);
    m_updateHashesState->addTransition(this, SIGNAL(finished(QDateTime)), m_finalState);

    m_sm->addState(m_sourceHashesState);
    m_sm->addState(m_destinationHashesState);
    m_sm->addState(m_syncState);
    m_sm->addState(m_updateHashesState);
    m_sm->addState(m_finalState);
    m_sm->setInitialState(m_sourceHashesState);

    connect(this, SIGNAL(started()), m_sm, SLOT(start()));
    connect(this, SIGNAL(error(QString)), m_sm, SLOT(stop()));
    connect(this, SIGNAL(synced()), SLOT(dbSynced()), Qt::QueuedConnection);

    connect(m_source, SIGNAL(syncDataAvailable(QVector<TimeLogSyncDataEntry>,
                                               QVector<TimeLogSyncDataCategory>,QDateTime)),
            this, SLOT(sourceDataAvailable(QVector<TimeLogSyncDataEntry>,
                                           QVector<TimeLogSyncDataCategory>,QDateTime)));
    connect(m_source, SIGNAL(hashesAvailable(QMap<QDateTime,QByteArray>)),
            this, SLOT(sourceHashesAvailable(QMap<QDateTime,QByteArray>)));

    connect(m_destination, SIGNAL(hashesAvailable(QMap<QDateTime,QByteArray>)),
            this, SLOT(destinationHashesAvailable(QMap<QDateTime,QByteArray>)));
    connect(m_destination, SIGNAL(dataSynced(QDateTime)),
            this, SLOT(destinationDataSynced(QDateTime)));
    connect(m_destination, SIGNAL(hashesUpdated()),
            this, SLOT(destinationHashesUpdated()));
}

void DBSyncer::start(bool isRecalcHashes, const QDateTime &maxMonth)
{
    m_maxMonth = maxMonth;
    m_isRecalcHashes = isRecalcHashes;
    m_latestMTime = QDateTime();

    emit started(QPrivateSignal());

    m_source->getHashes(m_maxMonth);
}

void DBSyncer::sourceDataAvailable(QVector<TimeLogSyncDataEntry> entryData,
                                   QVector<TimeLogSyncDataCategory> categoryData,
                                   QDateTime until)
{
    Q_UNUSED(until)

    if (!m_syncState->active()) {
        return;
    }

    QVector<TimeLogSyncDataEntry> updatedData;
    QVector<TimeLogSyncDataEntry> removedData;

    for (const TimeLogSyncDataEntry &entry: entryData) {
        if (entry.entry.isValid()) {
            updatedData.append(entry);
        } else {
            removedData.append(entry);
        }
    }

    m_destination->sync(updatedData, removedData, categoryData);
}

void DBSyncer::sourceHashesAvailable(QMap<QDateTime, QByteArray> hashes)
{
    if (!m_sourceHashesState->active()) {
        return;
    }

    m_sourceHashes = hashes;

    emit sourceHashesChecked(QPrivateSignal());

    m_destination->getHashes();
}

void DBSyncer::destinationHashesAvailable(QMap<QDateTime, QByteArray> hashes)
{
    if (!m_destinationHashesState->active()) {
        return;
    }

    m_destinationHashes = hashes;

    emit destinationHashesChecked(QPrivateSignal());

    m_syncPeriods = periodsToSync(m_sourceHashes, m_destinationHashes);

    if (m_syncPeriods.isEmpty()) {
        qCDebug(DB_SYNCER_CATEGORY) << "No pertiods to sync";
    } else {
        qCDebug(DB_SYNCER_CATEGORY) << "Periods to sync:" << m_syncPeriods;
    }

    if (m_syncPeriods.isEmpty()) {
        emit synced(QPrivateSignal());
    } else {
        syncNextPeriod();
    }
}

void DBSyncer::destinationDataSynced(const QDateTime &maxSyncDate)
{
    if (!m_syncState->active()) {
        return;
    }

    if (maxSyncDate > m_latestMTime) {
        m_latestMTime = maxSyncDate;
    }

    if (m_syncPeriods.isEmpty()) {
        emit synced(QPrivateSignal());
    } else {
        syncNextPeriod();
    }
}

void DBSyncer::destinationHashesUpdated()
{
    if (!m_updateHashesState->active()) {
        return;
    }

    emit finished(m_latestMTime);
}

void DBSyncer::dbSynced()
{
    if (m_isRecalcHashes) {
        m_destination->updateHashes();
    } else {
        emit finished(m_latestMTime);
    }
}

QList<QDateTime> DBSyncer::periodsToSync(const QMap<QDateTime, QByteArray> &source, const QMap<QDateTime, QByteArray> &destination) const
{
    QSet<QDateTime> srcPeriods(source.keys().toSet());
    QSet<QDateTime> destPeriods(destination.keys().toSet());

    QList<QDateTime> result;
    for (const QDateTime &start: QSet<QDateTime>(srcPeriods).intersect(destPeriods).toList()) {
        if (source.value(start) != destination.value(start)) {
            result.append(start);
        }
    }
    result.append(QSet<QDateTime>(srcPeriods).subtract(destPeriods).toList());

    std::sort(result.begin(), result.end());
    if (m_maxMonth.isValid()) {
        while (!result.isEmpty() && result.constLast() > m_maxMonth) {
            result.takeLast();
        }
    }

    return result;
}

void DBSyncer::syncNextPeriod()
{
    QDateTime begin(m_syncPeriods.takeLast());
    qCDebug(DB_SYNCER_CATEGORY) << "Syncyng for period" << begin;
    m_source->getSyncData(begin, begin.addMonths(1).addMSecs(-1));
}
