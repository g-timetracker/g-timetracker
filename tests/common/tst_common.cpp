#include <functional>

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

QVector<QDateTime> defaultMTimeSet = QVector<QDateTime>()
        << QDateTime::fromString("2015-11-01T11:00:00Z", Qt::ISODate)
        << QDateTime::fromString("2015-11-30T23:59:59Z", Qt::ISODate).addMSecs(999)
        << QDateTime::fromString("2015-12-01T00:00:00Z", Qt::ISODate)
        << QDateTime::fromString("2015-12-01T00:00:00Z", Qt::ISODate).addMSecs(1)
        << QDateTime::fromString("2015-12-01T00:00:00Z", Qt::ISODate).addMSecs(999)
        << QDateTime::fromString("2015-12-01T00:00:01Z", Qt::ISODate);

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

void extractSyncData(TimeLogHistory *history, QVector<TimeLogSyncData> &syncData)
{
    QSignalSpy historySyncDataSpy(history, SIGNAL(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)));
    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));
    history->getSyncData();
    QVERIFY(historySyncDataSpy.wait());
    QVERIFY(historyErrorSpy.isEmpty());
    syncData = historySyncDataSpy.constFirst().at(0).value<QVector<TimeLogSyncData> >();
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

void checkDB(TimeLogHistory *history, const QVector<TimeLogSyncData> &data)
{
    QVector<TimeLogSyncData> syncData;
    checkFunction(extractSyncData, history, syncData);
    QVERIFY(compareData(syncData, data));
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
    QVector<TimeLogSyncData> data;
    checkFunction(extractSyncData, history, data);

    QMap<QDateTime,QByteArray> hashes;
    checkFunction(extractHashes, history, hashes, noUpdate);
    checkFunction(verifyHashes, hashes);
    checkFunction(compareHashes, hashes, calcHashes(data));
}

const QVector<TimeLogEntry> &defaultData()
{
    return defaultDataset;
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

QVector<TimeLogSyncData> genSyncData(const QVector<TimeLogEntry> &data, const QVector<QDateTime> &mTimes)
{
    QVector<TimeLogSyncData> syncData;
    syncData.reserve(data.size());

    for (int i = 0; i < data.size(); i++) {
        syncData.append(TimeLogSyncData(data.at(i), mTimes.at(i)));
    }

    return syncData;
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

void updateDataSet(QVector<TimeLogSyncData> &data, const TimeLogSyncData &entry)
{
    data.erase(std::remove_if(data.begin(), data.end(), [&entry](const TimeLogSyncData &e) {
        return e.uuid == entry.uuid;
    }), data.end());
    auto it = std::lower_bound(data.begin(), data.end(), entry, [](const TimeLogSyncData &e1, const TimeLogSyncData &e2) {
        if (e1.mTime < e2.mTime) {
            return true;
        } else if (e1.mTime > e2.mTime) {
            return false;
        } else if (e1.isValid() && e2.isValid()) {
            return e1.startTime < e2.startTime;
        } else {
            return e1.isValid() > e2.isValid();
        }
    });
    data.insert(it, entry);
}

QMap<QDateTime, QByteArray> calcHashes(const QVector<TimeLogSyncData> &data)
{
    QMap<QDateTime, QByteArray> result;

    QDateTime periodStart;
    QByteArray hashData;
    QDataStream dataStream(&hashData, QIODevice::WriteOnly);

    QVector<TimeLogSyncData> sortedData(data);
    std::sort(sortedData.begin(), sortedData.end(), [](const TimeLogSyncData &d1, const TimeLogSyncData &d2) {
        if (d1.mTime == d2.mTime) {
            return d1.uuid.toRfc4122() < d2.uuid.toRfc4122();
        } else {
            return d1.mTime < d2.mTime;
        }
    });

    if (!sortedData.empty()) {
        periodStart = monthStart(sortedData.constFirst().mTime);
    }

    for (const TimeLogSyncData &entry: sortedData) {
        if (monthStart(entry.mTime) != periodStart) {
            if (!hashData.isEmpty()) {
                result.insert(periodStart, QCryptographicHash::hash(hashData, QCryptographicHash::Md5));
            }

            dataStream.device()->seek(0);
            hashData.clear();
            periodStart = monthStart(entry.mTime);
        }

        dataStream << entry.mTime.toMSecsSinceEpoch() << entry.uuid.toRfc4122();
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

void importSyncData(TimeLogHistory *history, const QVector<TimeLogSyncData> &data, int portionSize)
{
    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));
    QSignalSpy historyOutdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy historySyncSpy(history, SIGNAL(dataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)));

    int importedSize = 0;
    while (importedSize < data.size()) {
        int importSize = qMin(portionSize, data.size() - importedSize);
        QVector<TimeLogSyncData> updatedData, removedData;
        for (int i = 0; i < importSize; i++) {
            const TimeLogSyncData &entry = data.at(i + importedSize);
            if (entry.isValid()) {
                updatedData.append(entry);
            } else {
                removedData.append(entry);
            }
        }
        historySyncSpy.clear();
        history->sync(updatedData, removedData);
        QVERIFY(historySyncSpy.wait());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());

        importedSize += importSize;
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

bool compareData(const TimeLogSyncData &t1, const TimeLogSyncData &t2)
{
    if (t1.mTime != t2.mTime
        || !compareData(static_cast<TimeLogEntry>(t1), static_cast<TimeLogEntry>(t2))) {
        qCritical() << "Data does not match" << endl << t1 << endl << t2 << endl;
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

    for (int i = 0; i < d1.size() - 1; i++) {
        if (!compareData(d1.at(i), d2.at(i))) {
            dumpData(d1);
            dumpData(d2);
            return false;
        }
    }

    return true;
}

template bool compareData(const QVector<TimeLogEntry> &d1, const QVector<TimeLogEntry> &d2);
template bool compareData(const QVector<TimeLogSyncData> &d1, const QVector<TimeLogSyncData> &d2);

void compareHashes(const QMap<QDateTime, QByteArray> &h1, const QMap<QDateTime, QByteArray> &h2)
{
    checkFunction(verifyHashes, h1);
    QCOMPARE(h1, h2);
}
