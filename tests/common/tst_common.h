#ifndef COMMON_H
#define COMMON_H

#include <QtTest/QtTest>

#include "TimeLogHistory.h"
#include "TimeLogEntry.h"

#define checkFunction(func, ...) do {   \
    func(__VA_ARGS__);                  \
    if (QTest::currentTestFailed()) {   \
        QFAIL("Subtest failed");        \
    }                                   \
} while (0)

void checkInsert(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index);
void checkRemove(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index);
void checkEdit(QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, TimeLogHistory::Fields fields, int index);

void extractSyncData(TimeLogHistory *history, QVector<TimeLogSyncData> &syncData);
void extractHashes(TimeLogHistory *history, QMap<QDateTime, QByteArray> &hashes, bool noUpdate);
void checkDB(TimeLogHistory *history, const QVector<TimeLogEntry> &data);
void checkDB(TimeLogHistory *history, const QVector<TimeLogSyncData> &data);
void verifyHashes(const QMap<QDateTime, QByteArray> &hashes);
void checkHashesUpdated(const QMap<QDateTime, QByteArray> &hashes, bool isUpdated = true);
void checkHashesUpdated(TimeLogHistory *history, bool isUpdated = true);
void checkHashes(TimeLogHistory *history, const QMap<QDateTime, QByteArray> &origHashes);
void checkHashes(TimeLogHistory *history, bool noUpdate);

const QVector<TimeLogEntry> &defaultData();
const QVector<QDateTime> &defaultMTimes();
QStringList genCategories(int count);
QVector<TimeLogEntry> genData(int count);
QVector<TimeLogSyncData> genSyncData(const QVector<TimeLogEntry> &data, const QVector<QDateTime> &mTimes);
void updateDataSet(QVector<TimeLogEntry> &data, const TimeLogEntry &entry);
void updateDataSet(QVector<TimeLogSyncData> &data, const TimeLogSyncData &entry);
QMap<QDateTime, QByteArray> calcHashes(const QVector<TimeLogSyncData> &data);

QDateTime monthStart(const QDateTime &time);

void importSyncData(TimeLogHistory *history, const QVector<TimeLogSyncData> &updatedData, int portionSize);

template <typename T>
void dumpData(const QVector<T> &data);
bool checkData(const QVector<TimeLogEntry> &data);

bool compareData(const TimeLogEntry &t1, const TimeLogEntry &t2);
bool compareData(const TimeLogSyncData &t1, const TimeLogSyncData &t2);
template <typename T>
bool compareData(const QVector<T> &d1, const QVector<T> &d2);
extern template bool compareData(const QVector<TimeLogEntry> &d1, const QVector<TimeLogEntry> &d2);
extern template bool compareData(const QVector<TimeLogSyncData> &d1, const QVector<TimeLogSyncData> &d2);
void compareHashes(const QMap<QDateTime, QByteArray> &h1, const QMap<QDateTime, QByteArray> &h2);

#endif // COMMON_H
