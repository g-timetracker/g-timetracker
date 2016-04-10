#ifndef DBSYNCER_H
#define DBSYNCER_H

#include <QObject>

#include "TimeLogSyncDataEntry.h"
#include "TimeLogSyncDataCategory.h"

class QStateMachine;
class QState;
class QFinalState;

class TimeLogHistory;

class DBSyncer : public QObject
{
    Q_OBJECT
public:
    explicit DBSyncer(TimeLogHistory *source, TimeLogHistory *destination, QObject *parent = 0);

signals:
    void finished(QDateTime latestMTime) const;
    void error(const QString &errorText) const;

    void started(QPrivateSignal);
    void sourceHashesChecked(QPrivateSignal);
    void destinationHashesChecked(QPrivateSignal);
    void synced(QPrivateSignal);

public slots:
    void start(bool isRecalcHashes, const QDateTime &maxMonth = QDateTime());

private slots:
    void sourceDataAvailable(QVector<TimeLogSyncDataEntry> entryData,
                             QVector<TimeLogSyncDataCategory> categoryData, QDateTime until);
    void sourceHashesAvailable(QMap<QDateTime, QByteArray> hashes);

    void destinationHashesAvailable(QMap<QDateTime, QByteArray> hashes);
    void destinationDataSynced(QVector<TimeLogSyncDataEntry> updatedData,
                               QVector<TimeLogSyncDataEntry> removedData);
    void destinationHashesUpdated();

    void dbSynced();

private:
    QList<QDateTime> periodsToSync(const QMap<QDateTime, QByteArray> &source,
                                   const QMap<QDateTime, QByteArray> &destination) const;
    void syncNextPeriod();

    TimeLogHistory *m_source;
    TimeLogHistory *m_destination;

    QStateMachine *m_sm;
    QState *m_sourceHashesState;
    QState *m_destinationHashesState;
    QState *m_syncState;
    QState *m_updateHashesState;
    QFinalState *m_finalState;

    QDateTime m_maxMonth;
    bool m_isRecalcHashes;
    QMap<QDateTime, QByteArray> m_sourceHashes;
    QMap<QDateTime, QByteArray> m_destinationHashes;
    QList<QDateTime> m_syncPeriods;
    QDateTime m_latestMTime;
};

#endif // DBSYNCER_H
