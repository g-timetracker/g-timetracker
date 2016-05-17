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

#include <functional>
#include <random>

#include "tst_common.h"

namespace {
struct HashInput {
    QByteArray uuid;
    QDateTime mTime;
};
}

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

QVector<QDateTime> defaultMTimeSet = QVector<QDateTime>()
        << QDateTime::fromString("2015-11-01T11:00:00Z", Qt::ISODate)
        << QDateTime::fromString("2015-11-30T23:59:59Z", Qt::ISODate).addMSecs(999)
        << QDateTime::fromString("2015-12-01T00:00:00Z", Qt::ISODate)
        << QDateTime::fromString("2015-12-01T00:00:00Z", Qt::ISODate).addMSecs(1)
        << QDateTime::fromString("2015-12-01T00:00:00Z", Qt::ISODate).addMSecs(999)
        << QDateTime::fromString("2015-12-01T00:00:01Z", Qt::ISODate);

QVector<TimeLogCategory> defaultCategorySet = QVector<TimeLogCategory>()
        << TimeLogCategory(QUuid::createUuid(), TimeLogCategoryData("Category0"))
        << TimeLogCategory(QUuid::createUuid(), TimeLogCategoryData("Category1"))
        << TimeLogCategory(QUuid::createUuid(), TimeLogCategoryData("Category2"))
        << TimeLogCategory(QUuid::createUuid(), TimeLogCategoryData("Category3"))
        << TimeLogCategory(QUuid::createUuid(), TimeLogCategoryData("Category4"))
        << TimeLogCategory(QUuid::createUuid(), TimeLogCategoryData("Category5"));

const int minDuration = 1;
const int maxDuration = 20000;

const int minCategories = 1;
const int maxCategories = 100;

const int minDepth = 1;
const int maxDepth = 5;

const QString separator = ">";

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
        QCOMPARE(updateData.at(0).precedingStart, QDateTime::fromTime_t(0, Qt::UTC));
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
            QCOMPARE(updateData.at(0).precedingStart, QDateTime::fromTime_t(0, Qt::UTC));
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

void extractSyncData(TimeLogHistory *history,
                     QVector<TimeLogSyncDataEntry> &entryData,
                     QVector<TimeLogSyncDataCategory> &categoryData)
{
    QSignalSpy historySyncDataSpy(history, SIGNAL(syncDataAvailable(QVector<TimeLogSyncDataEntry>,QVector<TimeLogSyncDataCategory>,QDateTime)));
    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));
    history->getSyncData();
    QVERIFY(historySyncDataSpy.wait());
    QVERIFY(historyErrorSpy.isEmpty());
    entryData = historySyncDataSpy.constFirst().at(0).value<QVector<TimeLogSyncDataEntry> >();
    categoryData = historySyncDataSpy.constFirst().at(1).value<QVector<TimeLogSyncDataCategory> >();
}

void extractHashes(TimeLogHistory *history, QMap<QDateTime, QByteArray> &hashes, bool noUpdate)
{
    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));
    QSignalSpy historyHashesSpy(history, SIGNAL(hashesAvailable(QMap<QDateTime,QByteArray>)));
    history->getHashes(QDateTime(), noUpdate);
    QVERIFY(historyHashesSpy.wait());
    QVERIFY(historyErrorSpy.isEmpty());

    hashes = historyHashesSpy.constFirst().at(0).value<QMap<QDateTime,QByteArray> >();
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

void checkDB(TimeLogHistory *history, const QVector<TimeLogCategory> &data)
{
    QSignalSpy historyCategoriesSpy(history, SIGNAL(storedCategoriesAvailable(QVector<TimeLogCategory>)));
    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));
    history->getStoredCategories();
    QVERIFY(historyCategoriesSpy.wait());
    QVERIFY(historyErrorSpy.isEmpty());
    QVector<TimeLogCategory> categoryData = historyCategoriesSpy.constFirst().at(0).value<QVector<TimeLogCategory> >();

    QVector<TimeLogCategory> origData(data);
    std::sort(origData.begin(), origData.end(), [](const TimeLogCategoryData &d1, const TimeLogCategoryData &d2) {
        return d1.name < d2.name;
    });

    QVERIFY(compareData(categoryData, origData));
}

void checkDB(TimeLogHistory *history,
             const QVector<TimeLogSyncDataEntry> &entryData,
             const QVector<TimeLogSyncDataCategory> &categoryData)
{
    QVector<TimeLogSyncDataEntry> syncEntryData;
    QVector<TimeLogSyncDataCategory> syncCategoryData;
    checkFunction(extractSyncData, history, syncEntryData, syncCategoryData);

    QVERIFY(compareData(syncEntryData, entryData));

    auto syncCategoryCompare = [](const TimeLogSyncDataCategory &d1, const TimeLogSyncDataCategory &d2)
    {
        if (d1.sync.mTime < d2.sync.mTime) {
            return true;
        } else if (d1.sync.mTime > d2.sync.mTime) {
            return false;
        } else if (d1.category.isValid() && d2.category.isValid()) {
            return d1.category.name < d2.category.name;
        } else {
            return d1.category.isValid() > d2.category.isValid();
        }
    };
    QVector<TimeLogSyncDataCategory> origCategoryData(categoryData);
    std::sort(origCategoryData.begin(), origCategoryData.end(), syncCategoryCompare);
    std::sort(syncCategoryData.begin(), syncCategoryData.end(), syncCategoryCompare);

    QVERIFY(compareData(syncCategoryData, origCategoryData));
}

void verifyHashes(const QMap<QDateTime, QByteArray> &hashes)
{
    for (auto it = hashes.cbegin(); it != hashes.cend(); it++) {
        const QDateTime hashDate(it.key().toUTC());
        QVERIFY(hashDate.isValid());
        QCOMPARE(hashDate.date().day(), 1);
        QCOMPARE(hashDate.time().msecsSinceStartOfDay(), 0);
    }
}

void checkHashesUpdated(const QMap<QDateTime, QByteArray> &hashes, bool isUpdated)
{
    for (auto it = hashes.cbegin(); it != hashes.cend(); it++) {
        const QDateTime hashDate(it.key().toUTC());
        QVERIFY(hashDate.isValid());
        QCOMPARE(hashDate.date().day(), 1);
        QCOMPARE(hashDate.time().msecsSinceStartOfDay(), 0);
        QVERIFY(it.value().isEmpty() == !isUpdated);
    }
}

void checkHashesUpdated(TimeLogHistory *history, bool isUpdated)
{
    QMap<QDateTime,QByteArray> hashes;
    checkFunction(extractHashes, history, hashes, true);
    checkFunction(checkHashesUpdated, hashes, isUpdated);
}

void checkHashes(TimeLogHistory *history, const QMap<QDateTime, QByteArray> &origHashes)
{
    QMap<QDateTime,QByteArray> hashes;
    checkFunction(extractHashes, history, hashes, true);
    checkFunction(compareHashes, hashes, origHashes);
}

void checkHashes(TimeLogHistory *history, bool noUpdate)
{
    QVector<TimeLogSyncDataEntry> entryData;
    QVector<TimeLogSyncDataCategory> categoryData;
    checkFunction(extractSyncData, history, entryData, categoryData);

    QMap<QDateTime,QByteArray> hashes;
    checkFunction(extractHashes, history, hashes, noUpdate);
    checkFunction(verifyHashes, hashes);
    checkFunction(compareHashes, hashes, calcHashes(entryData, categoryData));
}

const QVector<TimeLogEntry> &defaultEntries()
{
    return defaultDataset;
}

const QVector<TimeLogCategory> &defaultCategories()
{
    return defaultCategorySet;
}

const QVector<QDateTime> &defaultMTimes()
{
    return defaultMTimeSet;
}

QStringList genCategories(int count)
{
    QVector<QStringList> categories(maxDepth - minDepth + 1);

    int categoriesLeft(count);
    QString prefix;

    for (int depth = minDepth-1; depth < maxDepth; depth++) {
        int categoriesCount = std::ceil(count / (maxDepth - minDepth + 1.0));

        for (int i = 0; i < categoriesCount && categoriesLeft; i++, categoriesLeft--) {
            QString category = QString("%1Category%2").arg(prefix).arg(i);

            if (depth) {
                category.prepend(QString("%1 %2 ").arg(categories.at(depth-1).at(i)).arg(separator));
            }

            categories[depth].append(category);
        }

        prefix.append("Sub");
    }

    QStringList result;

    for (int i = 0; i < categories.size(); i++) {
        result.append(categories.at(i));
    }

    return result;
}

QVector<TimeLogEntry> genData(int count)
{
    if (count == 0) {
        return QVector<TimeLogEntry>();
    }

    int categoryCount = std::ceil(std::log(count + 1));
    int categoriesCount = count / categoryCount;
    if (categoriesCount < minCategories) {
        categoriesCount = minCategories;
    } else if (categoriesCount > maxCategories) {
        categoriesCount = maxCategories;
    }
    categoryCount = std::ceil(count / categoriesCount);
    QStringList categories = genCategories(categoriesCount);

    QVector<TimeLogEntry> result;

    QDateTime startDate = QDateTime::currentDateTimeUtc().addSecs(-(maxDuration * count));

    std::uniform_int_distribution<> durationDistribution(minDuration, maxDuration);
    std::function<int()> randomDuration = std::bind(durationDistribution, std::default_random_engine());

    std::uniform_int_distribution<> categoryDistribution(0, categories.size() - 1);
    std::function<int()> randomCategory = std::bind(categoryDistribution, std::default_random_engine());

    for (int i = 0; i < count; i++) {
        TimeLogData data(startDate, categories.at(randomCategory()), "");
        TimeLogEntry entry(QUuid::createUuid(), data);
        result.append(entry);
        startDate = startDate.addSecs(randomDuration());
    }

    return result;
}

template <typename In, typename Out>
QVector<Out> genSyncData(const QVector<In> &data, const QVector<QDateTime> &mTimes)
{
    QVector<Out> syncData;
    syncData.reserve(data.size());

    for (int i = 0; i < data.size(); i++) {
        syncData.append(Out(data.at(i), mTimes.at(i)));
    }

    return syncData;
}

QVector<TimeLogSyncDataEntry> genSyncData(const QVector<TimeLogEntry> &data, const QVector<QDateTime> &mTimes)
{
    return genSyncData<TimeLogEntry, TimeLogSyncDataEntry>(data, mTimes);
}

QVector<TimeLogSyncDataCategory> genSyncData(const QVector<TimeLogCategory> &data, const QVector<QDateTime> &mTimes)
{
    return genSyncData<TimeLogCategory, TimeLogSyncDataCategory>(data, mTimes);
}

void updateDataSet(QVector<TimeLogEntry> &data, const TimeLogEntry &entry)
{
    data.erase(std::remove_if(data.begin(), data.end(), [&entry](const TimeLogEntry &e) {
        return e.uuid == entry.uuid;
    }), data.end());
    if (entry.isValid()) {
        auto it = std::lower_bound(data.begin(), data.end(), entry, [](const TimeLogEntry &e1, const TimeLogEntry &e2) {
            return e1.startTime < e2.startTime;
        });
        data.insert(it, entry);
    }
}

void updateDataSet(QVector<TimeLogCategory> &data, const TimeLogCategory &category)
{
    data.erase(std::remove_if(data.begin(), data.end(), [&category](const TimeLogCategory &c) {
        return c.uuid == category.uuid;
    }), data.end());
    if (category.isValid()) {
        data.append(category);
    }
}

void updateDataSet(QVector<TimeLogSyncDataEntry> &data, const TimeLogSyncDataEntry &entry)
{
    data.erase(std::remove_if(data.begin(), data.end(), [&entry](const TimeLogSyncDataEntry &e) {
        return e.entry.uuid == entry.entry.uuid;
    }), data.end());
    auto it = std::lower_bound(data.begin(), data.end(), entry, [](const TimeLogSyncDataEntry &e1, const TimeLogSyncDataEntry &e2) {
        if (e1.sync.mTime < e2.sync.mTime) {
            return true;
        } else if (e1.sync.mTime > e2.sync.mTime) {
            return false;
        } else if (e1.entry.isValid() && e2.entry.isValid()) {
            return e1.entry.startTime < e2.entry.startTime;
        } else {
            return e1.entry.isValid() > e2.entry.isValid();
        }
    });
    data.insert(it, entry);
}

void updateDataSet(QVector<TimeLogSyncDataCategory> &data, const TimeLogSyncDataCategory &category)
{
    data.erase(std::remove_if(data.begin(), data.end(), [&category](const TimeLogSyncDataCategory &c) {
        return c.category.uuid == category.category.uuid;
    }), data.end());
    auto it = std::lower_bound(data.begin(), data.end(), category, [](const TimeLogSyncDataCategory &c1, const TimeLogSyncDataCategory &c2) {
        if (c1.sync.mTime < c2.sync.mTime) {
            return true;
        } else if (c1.sync.mTime > c2.sync.mTime) {
            return false;
        } else if (c1.category.isValid() && c2.category.isValid()) {
            return c1.category.uuid < c2.category.uuid;
        } else {
            return c1.category.isValid() > c2.category.isValid();
        }
    });
    data.insert(it, category);
}

QMap<QDateTime, QByteArray> calcHashes(const QVector<TimeLogSyncDataEntry> &entryData,
                                       const QVector<TimeLogSyncDataCategory> &categoryData)
{
    QMap<QDateTime, QByteArray> result;

    QDateTime periodStart;
    QByteArray hashData;
    QDataStream dataStream(&hashData, QIODevice::WriteOnly);

    QVector<HashInput> sortedData;
    sortedData.reserve(entryData.size() + categoryData.size());
    for (const TimeLogSyncDataEntry &item: entryData) {
        HashInput input;
        input.uuid = item.entry.uuid.toRfc4122();
        input.mTime = item.sync.mTime;

        sortedData.append(input);
    }
    for (const TimeLogSyncDataCategory &item: categoryData) {
        HashInput input;
        input.uuid = item.category.uuid.toRfc4122();
        input.mTime = item.sync.mTime;

        sortedData.append(input);
    }

    std::sort(sortedData.begin(), sortedData.end(), [](const HashInput &i1, const HashInput &i2) {
        if (i1.mTime == i2.mTime) {
            return i1.uuid < i2.uuid;
        } else {
            return i1.mTime < i2.mTime;
        }
    });

    if (!sortedData.empty()) {
        periodStart = monthStart(sortedData.constFirst().mTime);
    }

    for (const HashInput &input: sortedData) {
        if (monthStart(input.mTime) != periodStart) {
            if (!hashData.isEmpty()) {
                result.insert(periodStart, QCryptographicHash::hash(hashData, QCryptographicHash::Md5));
            }

            dataStream.device()->seek(0);
            hashData.clear();
            periodStart = monthStart(input.mTime);
        }

        dataStream << input.mTime.toMSecsSinceEpoch() << input.uuid;
    }

    if (!hashData.isEmpty()) {
        result.insert(periodStart, QCryptographicHash::hash(hashData, QCryptographicHash::Md5));
    }

    return result;
}

QDateTime monthStart(const QDateTime &time)
{
    QDate date(time.toUTC().date());
    return QDateTime(QDate(date.year(), date.month(), 1), QTime(), Qt::UTC);
}

void importSyncData(TimeLogHistory *history,
                    const QVector<TimeLogSyncDataEntry> &entryData,
                    const QVector<TimeLogSyncDataCategory> &categoryData,
                    int portionSize)
{
    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));
    QSignalSpy historyOutdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy historySyncSpy(history, SIGNAL(dataSynced(QDateTime)));

    int importedSize = 0;
    int totalSize = entryData.size() + categoryData.size();
    while (importedSize < totalSize) {
        int currentPortionSize = qMin(portionSize, totalSize - importedSize);
        QVector<TimeLogSyncDataEntry> updatedPortion, removedPortion;
        QVector<TimeLogSyncDataCategory> categoryPortion;
        for (int i = 0; i < currentPortionSize; i++) {
            if (importedSize + i < entryData.size()) {
                const TimeLogSyncDataEntry &entry = entryData.at(i + importedSize);
                if (entry.entry.isValid()) {
                    updatedPortion.append(entry);
                } else {
                    removedPortion.append(entry);
                }
            } else {
                categoryPortion.append(categoryData.at(importedSize + i - entryData.size()));
            }
        }
        historySyncSpy.clear();
        history->sync(updatedPortion, removedPortion, categoryPortion);
        QVERIFY(historySyncSpy.wait());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());

        importedSize += currentPortionSize;
    }
}

template <typename T>
void dumpData(const QVector<T> &data)
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

bool compareData(const TimeLogCategory &c1, const TimeLogCategory &c2)
{
    if (c1.uuid != c2.uuid || c1.name != c2.name || c1.data != c2.data) {
        qCritical() << "Data does not match" << endl << c1 << endl << c2 << endl;
        return false;
    }

    return true;
}

bool compareData(const TimeLogSyncDataEntry &t1, const TimeLogSyncDataEntry &t2)
{
    if (t1.sync.mTime != t2.sync.mTime || !compareData(t1.entry, t2.entry)) {
        qCritical() << "Data does not match" << endl << t1 << endl << t2 << endl;
        return false;
    }

    return true;
}

bool compareData(const TimeLogSyncDataCategory &c1, const TimeLogSyncDataCategory &c2)
{
    if (c1.sync.mTime != c2.sync.mTime || !compareData(c1.category, c2.category)) {
        qCritical() << "Data does not match" << endl << c1 << endl << c2 << endl;
        return false;
    }

    return true;
}

template <typename T>
bool compareData(const QVector<T> &d1, const QVector<T> &d2)
{
    if (d1.size() != d2.size()) {
        qCritical() << "Sizes does not match" << d1.size() << d2.size();
        dumpData(d1);
        dumpData(d2);
        return false;
    }

    for (int i = 0; i < d1.size(); i++) {
        if (!compareData(d1.at(i), d2.at(i))) {
            dumpData(d1);
            dumpData(d2);
            return false;
        }
    }

    return true;
}

template bool compareData(const QVector<TimeLogEntry> &d1, const QVector<TimeLogEntry> &d2);
template bool compareData(const QVector<TimeLogCategory> &d1, const QVector<TimeLogCategory> &d2);
template bool compareData(const QVector<TimeLogSyncDataEntry> &d1, const QVector<TimeLogSyncDataEntry> &d2);
template bool compareData(const QVector<TimeLogSyncDataCategory> &d1, const QVector<TimeLogSyncDataCategory> &d2);

void compareHashes(const QMap<QDateTime, QByteArray> &h1, const QMap<QDateTime, QByteArray> &h2)
{
    checkFunction(verifyHashes, h1);
    QCOMPARE(h1, h2);
}
