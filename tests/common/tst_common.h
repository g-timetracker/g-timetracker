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

void checkDB(TimeLogHistory *history, const QVector<TimeLogEntry> &data);

const QVector<TimeLogEntry> &defaultData();
QStringList genCategories(int count);
QVector<TimeLogEntry> genData(int count);

void dumpData(const QVector<TimeLogEntry> &data);
bool checkData(const QVector<TimeLogEntry> &data);

bool compareData(const TimeLogEntry &t1, const TimeLogEntry &t2);
bool compareData(const QVector<TimeLogEntry> &d1, const QVector<TimeLogEntry> &d2);

#endif // COMMON_H
