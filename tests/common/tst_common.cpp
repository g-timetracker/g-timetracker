#include "tst_common.h"

QVector<TimeLogEntry> defaultDataset = QVector<TimeLogEntry>()
        << TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T11:00:00+0200",
                                                                               Qt::ISODate),
                                                         "Category0", ""))
        << TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T11:00:01+0200",
                                                                               Qt::ISODate),
                                                         "Category1", ""))
        << TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T11:59:59+0200",
                                                                               Qt::ISODate),
                                                         "Category2", ""))
        << TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T12:00:00+0200",
                                                                               Qt::ISODate),
                                                         "Category3", ""))
        << TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T23:59:59+0200",
                                                                               Qt::ISODate),
                                                         "Category4", ""))
        << TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-02T00:00:00+0200",
                                                                               Qt::ISODate),
                                                         "Category5", ""));

void checkInsert(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index)
{
    QVERIFY(actionSpy.size() == 1 || actionSpy.wait());
    QVERIFY(compareData(actionSpy.constFirst().at(0).value<TimeLogEntry>(), origData.at(index)));

    int resultIndex = 0;
    QVERIFY(!updateSpy.isEmpty() || updateSpy.wait());
    QVector<TimeLogEntry> updateData = updateSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();
    QVector<TimeLogHistory::Fields> updateFields = updateSpy.constFirst().at(1).value<QVector<TimeLogHistory::Fields> >();
    if (index != 0) {
        resultIndex = 1;
        QVERIFY(updateFields.at(1) & TimeLogHistory::PrecedingStart);
        QVERIFY(updateFields.at(0) & TimeLogHistory::DurationTime);
        QCOMPARE(updateData.at(1).precedingStart, updateData.at(0).startTime);
        QCOMPARE(updateData.at(0).durationTime, updateData.at(0).startTime.secsTo(updateData.at(1).startTime));
    } else {
        QCOMPARE(updateData.at(0).precedingStart, QDateTime::fromTime_t(0));
    }
    if (index < origData.size()-1) {
        resultIndex = updateData.size() - 2;
        QVERIFY(updateFields.at(resultIndex+1) & TimeLogHistory::PrecedingStart);
        QVERIFY(updateFields.at(resultIndex) & TimeLogHistory::DurationTime);
        QCOMPARE(updateData.at(resultIndex+1).precedingStart, updateData.at(resultIndex).startTime);
        QCOMPARE(updateData.at(resultIndex).durationTime, updateData.at(resultIndex).startTime.secsTo(updateData.at(resultIndex+1).startTime));
    } else {
        resultIndex = updateData.size() - 1;
        QCOMPARE(updateData.at(resultIndex).durationTime, -1);
    }
    QCOMPARE(updateData.at(resultIndex).startTime, origData.at(index).startTime);
    QVERIFY(updateFields.at(resultIndex) & (TimeLogHistory::PrecedingStart | TimeLogHistory::DurationTime));
}

void checkRemove(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index)
{
    QVERIFY(actionSpy.size() == 1 || actionSpy.wait());
    QCOMPARE(actionSpy.constFirst().at(0).value<TimeLogEntry>().uuid, origData.at(index).uuid);
    origData.remove(index);

    int resultIndex = 0;
    if (origData.size() == 0) {
        QVERIFY(updateSpy.isEmpty());
    } else {
        QVERIFY(!updateSpy.isEmpty() || updateSpy.wait());
        QVector<TimeLogEntry> updateData = updateSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();
        QVector<TimeLogHistory::Fields> updateFields = updateSpy.constFirst().at(1).value<QVector<TimeLogHistory::Fields> >();

        resultIndex = 0;
        if (index != 0) {
            QVERIFY(updateFields.at(0) & TimeLogHistory::DurationTime);
            if (index != origData.size()) {
                QCOMPARE(updateData.at(0).durationTime, updateData.at(0).startTime.secsTo(updateData.at(1).startTime));
            } else {
                QCOMPARE(updateData.at(0).durationTime, -1);
            }
        } else {
            QCOMPARE(updateData.at(0).precedingStart, QDateTime::fromTime_t(0));
        }
        if (index < origData.size()-1) {
            if (index != 0) {
                QVERIFY(updateFields.at(1) & TimeLogHistory::PrecedingStart);
                QCOMPARE(updateData.at(1).precedingStart, updateData.at(0).startTime);
            } else {
                QVERIFY(updateFields.at(0) & TimeLogHistory::PrecedingStart);
                QCOMPARE(updateData.at(0).precedingStart.toTime_t(), static_cast<uint>(0));
            }
        } else {
            resultIndex = updateData.size() - 1;
            QCOMPARE(updateData.at(resultIndex).durationTime, -1);
        }
        QVERIFY(updateFields.at(resultIndex) & (TimeLogHistory::PrecedingStart | TimeLogHistory::DurationTime));
    }
}

void checkEdit(QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, TimeLogHistory::Fields fields, int index)
{
    QVERIFY(!updateSpy.isEmpty() || updateSpy.wait());
    QVector<TimeLogEntry> updateData = updateSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();
    QVector<TimeLogHistory::Fields> updateFields = updateSpy.constFirst().at(1).value<QVector<TimeLogHistory::Fields> >();

    int resultIndex = 0;
    if (fields & TimeLogHistory::StartTime) {
        if (index != 0) {
            resultIndex = 1;
            QVERIFY(updateFields.at(0) & TimeLogHistory::PrecedingStart);
            QVERIFY(updateFields.at(1) & TimeLogHistory::DurationTime);
            QCOMPARE(updateData.at(1).precedingStart, updateData.at(0).startTime);
            QCOMPARE(updateData.at(0).durationTime, updateData.at(0).startTime.secsTo(updateData.at(1).startTime));
        }
        if (index != origData.size()-1) {
            resultIndex = updateData.size() - 2;
            QVERIFY(updateFields.at(resultIndex+1) & TimeLogHistory::PrecedingStart);
            QVERIFY(updateFields.at(resultIndex) & TimeLogHistory::DurationTime);
            QCOMPARE(updateData.at(resultIndex+1).precedingStart, updateData.at(resultIndex).startTime);
            QCOMPARE(updateData.at(resultIndex).durationTime, updateData.at(resultIndex).startTime.secsTo(updateData.at(resultIndex+1).startTime));
        }
        QCOMPARE(updateData.at(resultIndex).startTime, origData.at(index).startTime);
//        QEXPECT_FAIL("", "All flags are the same for all items", Continue);
//        QVERIFY(updateFields.at(resultIndex) == (fields | TimeLogHistory::DurationTime));   // FIXME
    }
    QVERIFY((updateFields.at(resultIndex) & TimeLogHistory::AllFieldsMask) == fields);
    if (fields & TimeLogHistory::Category) {
        QCOMPARE(updateData.at(resultIndex).category, origData.at(index).category);
    }
    if (fields & TimeLogHistory::Comment) {
        QCOMPARE(updateData.at(resultIndex).comment, origData.at(index).comment);
    }
}

void checkDB(TimeLogHistory *history, const QVector<TimeLogEntry> &data)
{
    QSignalSpy historyDataSpy(history, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));
    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));
    qlonglong id = QDateTime::currentMSecsSinceEpoch();
    history->getHistoryBetween(id);
    QVERIFY(historyDataSpy.wait());
    QVERIFY(historyErrorSpy.isEmpty());
    QCOMPARE(historyDataSpy.constFirst().at(1).toLongLong(), id);
    QVector<TimeLogEntry> historyData = historyDataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();
    QVERIFY(compareData(historyData, data));
}

const QVector<TimeLogEntry> &defaultData()
{
    return defaultDataset;
}

void dumpData(const QVector<TimeLogEntry> &data)
{
    for (int i = 0; i < data.size(); i++) {
        qDebug() << i << data.at(i) << endl;
    }
}

bool checkData(const QVector<TimeLogEntry> &data)
{
    for (int i = 0; i < data.size(); i++) {
        if (!data.at(i).isValid()) {
            qCritical() << "Data not valid at index" << i;
            dumpData(data);
            return false;
        }

        if (i > 0) {
            if (data.at(i).precedingStart != data.at(i-1).startTime) {
                qCritical() << "Wrong preceding start at index" << i;
                dumpData(data);
                return false;
            }
        } else if (data.at(i).precedingStart.toTime_t() != 0) {
            qCritical() << "Wrong preceding start at index" << i;
            dumpData(data);
            return false;
        }

        if (i < data.size()-1) {
            if (data.at(i).durationTime != data.at(i).startTime.secsTo(data.at(i+1).startTime)) {
                qCritical() << "Wrong duration at index" << i;
                dumpData(data);
                return false;
            }
        }
    }

    return true;
}

bool compareData(const TimeLogEntry &t1, const TimeLogEntry &t2)
{
    if (t1.uuid != t2.uuid || t1.startTime != t2.startTime || t1.category != t2.category
        || t1.comment != t2.comment) {
        qCritical() << "Data does not match" << endl << t1 << endl << t2 << endl;
        return false;
    }

    return true;
}

bool compareData(const QVector<TimeLogEntry> &d1, const QVector<TimeLogEntry> &d2)
{
    if (d1.size() != d2.size()) {
        qCritical() << "Sizes does not match" << d1.size() << d2.size();
        dumpData(d1);
        dumpData(d2);
        return false;
    }

    for (int i = 0; i < d1.size() - 1; i++) {
        if (!compareData(d1.at(i), d2.at(i))) {
            dumpData(d1);
            dumpData(d2);
            return false;
        }
    }

    return true;
}
