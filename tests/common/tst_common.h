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

void extractSyncData(TimeLogHistory *history, QVector<TimeLogSyncDataEntry> &entryData, QVector<TimeLogSyncDataCategory> &categoryData);
void extractHashes(TimeLogHistory *history, QMap<QDateTime, QByteArray> &hashes, bool noUpdate);
void checkDB(TimeLogHistory *history, const QVector<TimeLogEntry> &data);
void checkDB(TimeLogHistory *history, const QVector<TimeLogCategory> &data);
void checkDB(TimeLogHistory *history, const QVector<TimeLogSyncDataEntry> &entryData, const QVector<TimeLogSyncDataCategory> &categoryData);
void verifyHashes(const QMap<QDateTime, QByteArray> &hashes);
void checkHashesUpdated(const QMap<QDateTime, QByteArray> &hashes, bool isUpdated = true);
void checkHashesUpdated(TimeLogHistory *history, bool isUpdated = true);
void checkHashes(TimeLogHistory *history, const QMap<QDateTime, QByteArray> &origHashes);
void checkHashes(TimeLogHistory *history, bool noUpdate);

const QVector<TimeLogEntry> &defaultEntries();
const QVector<TimeLogCategory> &defaultCategories();
const QVector<QDateTime> &defaultMTimes();
QStringList genCategories(int count);
QVector<TimeLogEntry> genData(int count);
template <typename In, typename Out>
QVector<Out> genSyncData(const QVector<In> &data, const QVector<QDateTime> &mTimes);
QVector<TimeLogSyncDataEntry> genSyncData(const QVector<TimeLogEntry> &data, const QVector<QDateTime> &mTimes);
QVector<TimeLogSyncDataCategory> genSyncData(const QVector<TimeLogCategory> &data, const QVector<QDateTime> &mTimes);
void updateDataSet(QVector<TimeLogEntry> &data, const TimeLogEntry &entry);
void updateDataSet(QVector<TimeLogCategory> &data, const TimeLogCategory &category);
void updateDataSet(QVector<TimeLogSyncDataEntry> &data, const TimeLogSyncDataEntry &entry);
void updateDataSet(QVector<TimeLogSyncDataCategory> &data, const TimeLogSyncDataCategory &category);
QMap<QDateTime, QByteArray> calcHashes(const QVector<TimeLogSyncDataEntry> &entryData,
                                       const QVector<TimeLogSyncDataCategory> &categoryData);

QDateTime monthStart(const QDateTime &time);

void importSyncData(TimeLogHistory *history, const QVector<TimeLogSyncDataEntry> &entryData,
                    const QVector<TimeLogSyncDataCategory> &categoryData, int portionSize);

template <typename T>
void dumpData(const QVector<T> &data);
bool checkData(const QVector<TimeLogEntry> &data);

bool compareData(const TimeLogEntry &t1, const TimeLogEntry &t2);
bool compareData(const TimeLogCategory &c1, const TimeLogCategory &c2);
bool compareData(const TimeLogSyncDataEntry &t1, const TimeLogSyncDataEntry &t2);
bool compareData(const TimeLogSyncDataCategory &c1, const TimeLogSyncDataCategory &c2);
template <typename T>
bool compareData(const QVector<T> &d1, const QVector<T> &d2);
extern template bool compareData(const QVector<TimeLogEntry> &d1, const QVector<TimeLogEntry> &d2);
extern template bool compareData(const QVector<TimeLogCategory> &d1, const QVector<TimeLogCategory> &d2);
extern template bool compareData(const QVector<TimeLogSyncDataEntry> &d1, const QVector<TimeLogSyncDataEntry> &d2);
extern template bool compareData(const QVector<TimeLogSyncDataCategory> &d1, const QVector<TimeLogSyncDataCategory> &d2);
void compareHashes(const QMap<QDateTime, QByteArray> &h1, const QMap<QDateTime, QByteArray> &h2);

#endif // COMMON_H
