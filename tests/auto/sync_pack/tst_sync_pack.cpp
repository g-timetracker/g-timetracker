#include <QtTest/QtTest>

#include <QTemporaryDir>

#include "tst_common.h"
#include "DataSyncer.h"
#include "TimeLogCategoryTreeNode.h"

QLoggingCategory::CategoryFilter oldCategoryFilter;

void syncerCategoryFilter(QLoggingCategory *category)
{
    // Disable syncer info logs, as it's produces too much output
    if (qstrcmp(category->categoryName(), "DataSyncerWorker") == 0) {
        category->setEnabled(QtInfoMsg, false);
        category->setEnabled(QtDebugMsg, false);
    } else if (qstrcmp(category->categoryName(), "DataIO") == 0) {
        category->setEnabled(QtInfoMsg, false);
        category->setEnabled(QtDebugMsg, false);
    } else {
        oldCategoryFilter(category);
    }
}

QTemporaryDir *dataDir1 = nullptr;
QTemporaryDir *dataDir2 = nullptr;
QTemporaryDir *dataDir3 = Q_NULLPTR;
QTemporaryDir *syncDir1 = nullptr;
QTemporaryDir *syncDir2 = nullptr;
TimeLogHistory *history1 = nullptr;
TimeLogHistory *history2 = nullptr;
TimeLogHistory *history3 = nullptr;
DataSyncer *syncer1 = nullptr;
DataSyncer *syncer2 = nullptr;
DataSyncer *syncer3 = nullptr;

const int mTimeLength = QString::number(std::numeric_limits<qint64>::max()).length();
const QString fileNamePattern = QString("(?<mTime>\\d{%1})-\\{[\\w-]+\\}").arg(mTimeLength);

const QString syncFileNamePattern = QString("^%1\\.sync$").arg(fileNamePattern);
const QRegularExpression syncFileNameRegexp(syncFileNamePattern);

const QString packFileNamePattern = QString("^%1\\.pack$").arg(fileNamePattern);
const QRegularExpression packFileNameRegexp(packFileNamePattern);

void importSyncData(TimeLogHistory *history, DataSyncer *syncer, QTemporaryDir *syncDir,
                    const QVector<TimeLogSyncDataEntry> &entryData,
                    const QVector<TimeLogSyncDataCategory> &categoryData, int portionSize,
                    bool noPack = true, const QDateTime &syncTime = QDateTime::currentDateTimeUtc())
{
    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));
    QSignalSpy historyOutdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy historySyncSpy(history, SIGNAL(dataSynced(QVector<TimeLogSyncDataEntry>,QVector<TimeLogSyncDataEntry>)));

    QSignalSpy syncSpy(syncer, SIGNAL(synced()));
    QSignalSpy syncErrorSpy(syncer, SIGNAL(error(QString)));

    syncer->setNoPack(true);

    syncer->setSyncPath(QUrl::fromLocalFile(syncDir->path()));

    QVector<TimeLogSyncDataEntry> importEntryData(entryData);
    QVector<TimeLogSyncDataCategory> importCategoryData(categoryData);

    int importedSize = 0;
    int totalSize = entryData.size() + categoryData.size();
    while (importedSize < totalSize) {
        int currentPortionSize = qMin(portionSize, totalSize - importedSize);
        QVector<TimeLogSyncDataEntry> updatedPortion, removedPortion;
        QVector<TimeLogSyncDataCategory> categoryPortion;
        for (int i = 0; i < currentPortionSize; i++) {
            if (!importEntryData.isEmpty()
                && (importCategoryData.isEmpty()
                    || importEntryData.constFirst().sync.mTime < importCategoryData.constFirst().sync.mTime)) {
                const TimeLogSyncDataEntry &entry = importEntryData.takeFirst();
                if (entry.entry.isValid()) {
                    updatedPortion.append(entry);
                } else {
                    removedPortion.append(entry);
                }
            } else {
                categoryPortion.append(importCategoryData.takeFirst());
            }
        }
        historySyncSpy.clear();
        history->sync(updatedPortion, removedPortion, categoryPortion);
        QVERIFY(historySyncSpy.wait());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());

        importedSize += currentPortionSize;

        syncSpy.clear();
        syncer->sync(syncTime);
        QVERIFY(syncSpy.wait());
        QVERIFY(syncErrorSpy.isEmpty());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());
    }

    if (entryData.isEmpty() && categoryData.isEmpty()) {
        syncSpy.clear();
        syncer->sync(syncTime);
        QVERIFY(syncSpy.wait());
        QVERIFY(syncErrorSpy.isEmpty());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());
    }

    syncer->setNoPack(false);

    if (!noPack) {
        syncSpy.clear();
        syncer->sync(syncTime);
        QVERIFY(syncSpy.wait());
        QVERIFY(syncErrorSpy.isEmpty());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());
    }

    checkFunction(checkDB, history, entryData, categoryData);
}

QFileInfoList buildFileList(const QString &path, bool isRecursive = false,
                            QStringList filters = QStringList())
{
    QFileInfoList result;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return result;
    }

    if (fileInfo.isFile()) {
        result.append(path);
    } else if (fileInfo.isDir()) {
        QDir dir(path);
        QFileInfoList entries;
        entries = dir.entryInfoList(filters, QDir::Files);
        result.append(entries);
        if (isRecursive) {
            entries = dir.entryInfoList(filters, QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QFileInfo &entry: entries) {
                result.append(buildFileList(entry.filePath(), isRecursive, filters));
            }
        }
    }

    return result;
}

QVector<QDateTime> mTimeFileList(const QFileInfoList &infoList, const QRegularExpression &regexp)
{
    QVector<QDateTime> result;

    result.reserve(infoList.size());
    for (const QFileInfo &entry: infoList) {
        QString fileName = entry.fileName();
        QRegularExpressionMatch match(regexp.match(fileName));
        if (match.hasMatch()) {
            result.append(QDateTime::fromMSecsSinceEpoch(match.captured("mTime").toLongLong(), Qt::UTC));
        }
    }

    return result;
}

QVector<QDateTime> syncMTimeFileList(const QString &path)
{
    return mTimeFileList(buildFileList(path, false, QStringList() << "*.sync"), syncFileNameRegexp);
}

QVector<TimeLogSyncDataBase> syncDataBase(const QVector<TimeLogSyncDataEntry> &entryData,
                                          const QVector<TimeLogSyncDataCategory> &categoryData)
{
    QVector<TimeLogSyncDataEntry> importEntryData(entryData);
    QVector<TimeLogSyncDataCategory> importCategoryData(categoryData);
    QVector<TimeLogSyncDataBase> result;
    int totalSize = importEntryData.size() + importCategoryData.size();
    for (int i = 0; i < totalSize; i++) {
        if (!importEntryData.isEmpty()
            && (importCategoryData.isEmpty()
                || importEntryData.constFirst().sync.mTime < importCategoryData.constFirst().sync.mTime)) {
            result.append(importEntryData.takeFirst().sync);
        } else {
            result.append(importCategoryData.takeFirst().sync);
        }
    }

    return result;
}

QVector<QDateTime> syncMTimeList(const QVector<TimeLogSyncDataEntry> &entryData,
                                 const QVector<TimeLogSyncDataCategory> &categoryData,
                                 int syncPortion, const QDateTime &maxPacked)
{
    QVector<TimeLogSyncDataBase> syncData(syncDataBase(entryData, categoryData));

    QVector<QDateTime> result;
    for (int stepSize = qMin(syncPortion, syncData.size()), i = stepSize - 1;
         i >= 0 && i < syncData.size();
         stepSize = qMin(syncPortion, syncData.size() - i), i += stepSize) {
        if (maxPacked.isNull() || syncData.at(i).mTime > maxPacked) {
            result.append(syncData.at(i).mTime);
        }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

void checkSyncFolder(const QString &path, const QVector<TimeLogSyncDataEntry> &entryData,
                     const QVector<TimeLogSyncDataCategory> &categoryData, int syncPortion)
{
    QCOMPARE(syncMTimeFileList(path), syncMTimeList(entryData, categoryData, syncPortion, QDateTime()));
}

void checkPackHashes(const QString &path, const QString &file)
{
    QScopedPointer<TimeLogHistory> pack(new TimeLogHistory());
    QVERIFY(pack->init(path, file));
    checkHashesUpdated(pack.data(), true);
}

void checkPackFolder(const QString &path, const QVector<TimeLogSyncDataEntry> &entryData,
                     const QVector<TimeLogSyncDataCategory> &categoryData, int syncPortion,
                     const QDateTime &maxPacked, const QVector<QDateTime> &notPacked = QVector<QDateTime>())
{
    QVector<QDateTime> mTimes(notPacked);
    mTimes.erase(std::unique(mTimes.begin(), mTimes.end()), mTimes.end());
    mTimes += syncMTimeList(entryData, categoryData, syncPortion, maxPacked);
    std::sort(mTimes.begin(), mTimes.end());
    QCOMPARE(syncMTimeFileList(path), mTimes);

    QVector<TimeLogSyncDataBase> syncData(syncDataBase(entryData, categoryData));
    QDateTime packMTime;
    for (const TimeLogSyncDataBase &item: syncData) {
        if (item.mTime <= maxPacked) {
            packMTime = item.mTime;
        } else {
            break;
        }
    }

    QFileInfoList packList(buildFileList(path, false, QStringList() << "*.pack"));
    for (const QFileInfo &fileInfo: packList) {
        checkPackHashes(path, fileInfo.fileName());
    }

    QCOMPARE(mTimeFileList(packList, packFileNameRegexp),
             packMTime.isValid() ? QVector<QDateTime>() << packMTime : QVector<QDateTime>());
}

class tst_SyncPack : public QObject
{
    Q_OBJECT

public:
    tst_SyncPack();

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void pack();
    void pack_data();
    void syncPack();
    void syncPack_data();

    void unpacked();
    void unpacked_data();

    void twoPacks();
    void twoPacks_data();
};

tst_SyncPack::tst_SyncPack()
{
}

void tst_SyncPack::init()
{
    dataDir1 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir1);
    QVERIFY(dataDir1->isValid());
    history1 = new TimeLogHistory;
    Q_CHECK_PTR(history1);
    QVERIFY(history1->init(dataDir1->path()));
    syncer1 = new DataSyncer(history1);
    Q_CHECK_PTR(syncer1);
    syncer1->init(dataDir1->path());
    syncer1->setAutoSync(false);

    dataDir2 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir2);
    QVERIFY(dataDir2->isValid());
    history2 = new TimeLogHistory;
    Q_CHECK_PTR(history2);
    QVERIFY(history2->init(dataDir2->path()));
    syncer2 = new DataSyncer(history2);
    Q_CHECK_PTR(syncer2);
    syncer2->init(dataDir2->path());
    syncer2->setAutoSync(false);

    dataDir3 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir3);
    QVERIFY(dataDir3->isValid());
    history3 = new TimeLogHistory;
    Q_CHECK_PTR(history3);
    QVERIFY(history3->init(dataDir3->path()));
    syncer3 = new DataSyncer(history3);
    Q_CHECK_PTR(syncer3);
    syncer3->init(dataDir3->path());
    syncer3->setAutoSync(false);

    syncDir1 = new QTemporaryDir();
    Q_CHECK_PTR(syncDir1);
    QVERIFY(syncDir1->isValid());

    syncDir2= new QTemporaryDir();
    Q_CHECK_PTR(syncDir2);
    QVERIFY(syncDir2->isValid());
}

void tst_SyncPack::cleanup()
{
    if (QTest::currentTestFailed()) {
//        dataDir1->setAutoRemove(false);
//        dataDir2->setAutoRemove(false);
//        dataDir3->setAutoRemove(false);
//        syncDir1->setAutoRemove(false);
//        syncDir2->setAutoRemove(false);
    }

    delete syncer1;
    syncer1 = nullptr;
    delete history1;
    history1 = nullptr;
    delete dataDir1;
    dataDir1 = nullptr;

    delete syncer2;
    syncer2 = nullptr;
    delete history2;
    history2 = nullptr;
    delete dataDir2;
    dataDir2 = nullptr;

    delete syncer3;
    syncer3 = nullptr;
    delete history3;
    history3 = nullptr;
    delete dataDir3;
    dataDir3 = nullptr;

    delete syncDir1;
    syncDir1 = nullptr;

    delete syncDir2;
    syncDir2 = nullptr;
}

void tst_SyncPack::initTestCase()
{
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();
    qRegisterMetaType<QVector<TimeLogSyncDataEntry> >();
    qRegisterMetaType<QVector<TimeLogSyncDataCategory> >();
    qRegisterMetaType<QSharedPointer<TimeLogCategoryTreeNode> >();
    qRegisterMetaType<QMap<QDateTime,QByteArray> >();

    oldCategoryFilter = QLoggingCategory::installFilter(nullptr);
    QLoggingCategory::installFilter(syncerCategoryFilter);
    qSetMessagePattern("[%{time}] <%{category}> %{type} (%{file}:%{line}, %{function}) %{message}");
}

void tst_SyncPack::cleanupTestCase()
{
}

void tst_SyncPack::pack()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));
    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));
    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QFETCH(int, syncPortion);
    QFETCH(QDateTime, syncStart);

    checkFunction(importSyncData, history1, syncer1, syncDir1, origSyncEntries, origSyncCategories, syncPortion, true);

    checkFunction(checkSyncFolder, QDir(dataDir1->path()).filePath("sync"), origSyncEntries, origSyncCategories, syncPortion);

    // Pack
    syncSpy1.clear();
    syncer1->setSyncPath(QUrl::fromLocalFile(syncDir1->path()));
    syncer1->pack(syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, monthStart(syncStart).addMSecs(-1));

    // Sync 2 [in]
    syncer2->setSyncPath(QUrl::fromLocalFile(syncDir1->path()));
    syncer2->sync(syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, origEntries);
    checkFunction(checkDB, history2, origCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
    checkFunction(checkPackFolder, QDir(dataDir2->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, monthStart(syncStart).addMSecs(-1));

    // Sync 1 [in]
    syncer1->setSyncPath(QUrl::fromLocalFile(syncDir1->path()));
    syncer1->sync(syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origEntries);
    checkFunction(checkDB, history1, origCategories);
    checkFunction(checkDB, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, monthStart(syncStart).addMSecs(-1));
}

void tst_SyncPack::pack_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");
    QTest::addColumn<int>("syncPortion");
    QTest::addColumn<QDateTime>("syncStart");

    auto addTest = [](int size, int syncPortion, int packCount, const QDateTime &syncStart,
            const QString &info)
    {
        QTest::newRow(QString("%1 entries by %2, pack %3%4").arg(size).arg(syncPortion)
                      .arg(packCount).arg(info.isEmpty() ? "" : " " + info).toLocal8Bit())
                << size << 0 << syncPortion << syncStart;

        QTest::newRow(QString("%1 categories by %2, pack %3%4").arg(size).arg(syncPortion)
                      .arg(packCount).arg(info.isEmpty() ? "" : " " + info).toLocal8Bit())
                << 0 << size << syncPortion << syncStart;

        QTest::newRow(QString("%1 entries, %1 categories by %2, pack %3%4").arg(size)
                      .arg(syncPortion * 2).arg(packCount * 2)
                      .arg(info.isEmpty() ? "" : " " + info).toLocal8Bit())
                << size << size << syncPortion * 2 << syncStart;
    };

    addTest(0, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "empty");

    addTest(1, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "");
    addTest(1, 1, 1, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(1, 1, 1, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(1, 1, 1, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");

    addTest(2, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "");
    addTest(2, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(2, 2, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(2, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(2, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");
    addTest(2, 2, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(2, 2, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");

    addTest(4, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "");
    addTest(4, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(4, 1, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(4, 2, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(4, 4, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(4, 1, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(4, 1, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");
    addTest(4, 4, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(4, 4, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");

    addTest(6, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "");
    addTest(6, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(6, 1, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(6, 2, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(6, 3, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(6, 6, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(6, 1, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(6, 1, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");
    addTest(6, 6, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(6, 6, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");
}

void tst_SyncPack::syncPack()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));
    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));
    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QFETCH(int, syncPortion);
    QFETCH(QDateTime, syncStart);

    checkFunction(importSyncData, history1, syncer1, syncDir1, origSyncEntries, origSyncCategories, syncPortion, false, syncStart);

    // Sync 1 [out]
    syncer1->setSyncPath(QUrl::fromLocalFile(syncDir1->path()));
    syncer1->sync(syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origEntries);
    checkFunction(checkDB, history1, origCategories);
    checkFunction(checkDB, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, monthStart(syncStart).addMSecs(-1));

    // Sync 2 [in]
    syncer2->setSyncPath(QUrl::fromLocalFile(syncDir1->path()));
    syncer2->sync(syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, origEntries);
    checkFunction(checkDB, history2, origCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
    checkFunction(checkPackFolder, QDir(dataDir2->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, monthStart(syncStart).addMSecs(-1));

    // Sync 1 [in]
    syncer1->setSyncPath(QUrl::fromLocalFile(syncDir1->path()));
    syncer1->sync(syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origEntries);
    checkFunction(checkDB, history1, origCategories);
    checkFunction(checkDB, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, monthStart(syncStart).addMSecs(-1));
}

void tst_SyncPack::syncPack_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");
    QTest::addColumn<int>("syncPortion");
    QTest::addColumn<QDateTime>("syncStart");

    auto addTest = [](int size, int syncPortion, int packCount, const QDateTime &syncStart,
            const QString &info)
    {
        QTest::newRow(QString("%1 entries by %2, pack %3%4").arg(size).arg(syncPortion)
                      .arg(packCount).arg(info.isEmpty() ? "" : " " + info).toLocal8Bit())
                << size << 0 << syncPortion << syncStart;

        QTest::newRow(QString("%1 categories by %2, pack %3%4").arg(size).arg(syncPortion)
                      .arg(packCount).arg(info.isEmpty() ? "" : " " + info).toLocal8Bit())
                << 0 << size << syncPortion << syncStart;

        QTest::newRow(QString("%1 entries, %1 categories by %2, pack %3%4").arg(size)
                      .arg(syncPortion * 2).arg(packCount * 2)
                      .arg(info.isEmpty() ? "" : " " + info).toLocal8Bit())
                << size << size << syncPortion * 2 << syncStart;
    };

    addTest(0, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "empty");

    addTest(1, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "");
    addTest(1, 1, 1, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(1, 1, 1, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(1, 1, 1, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");

    addTest(2, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "");
    addTest(2, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(2, 2, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(2, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(2, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");
    addTest(2, 2, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(2, 2, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");

    addTest(4, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "");
    addTest(4, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(4, 1, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(4, 2, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(4, 4, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(4, 1, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(4, 1, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");
    addTest(4, 4, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(4, 4, 4, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");

    addTest(6, 1, 0, QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC), "");
    addTest(6, 1, 2, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC), "");
    addTest(6, 1, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(6, 2, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(6, 3, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(6, 6, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC), "");
    addTest(6, 1, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(6, 1, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");
    addTest(6, 6, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(-1), "-1 ms");
    addTest(6, 6, 6, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC).addMSecs(1), "+1 ms");
}

void tst_SyncPack::unpacked()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));
    QSignalSpy syncSpy3(syncer3, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy3(syncer3, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy3(history3, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy3(history3, SIGNAL(dataOutdated()));

    QFETCH(int, syncPortion);
    QFETCH(QDateTime, syncStart);

    checkFunction(importSyncData, history1, syncer1, syncDir1, origSyncEntries, origSyncCategories, syncPortion, false, syncStart);

    QFETCH(QVector<TimeLogSyncDataEntry>, newEntries);
    QFETCH(QVector<TimeLogSyncDataCategory>, newCategories);

    checkFunction(importSyncData, history2, newEntries, newCategories, syncPortion);

    QVector<TimeLogEntry> updatedEntries(origEntries);
    QVector<TimeLogSyncDataEntry> updatedSyncEntries(origSyncEntries);
    if (!newEntries.isEmpty()) {
        updateDataSet(updatedEntries, newEntries.constFirst().entry);
        updateDataSet(updatedSyncEntries, newEntries.constFirst());
    }
    QVector<TimeLogCategory> updatedCategories(origCategories);
    QVector<TimeLogSyncDataCategory> updatedSyncCategories(origSyncCategories);
    if (!newCategories.isEmpty()) {
        updateDataSet(updatedCategories, newCategories.constFirst().category);
        updateDataSet(updatedSyncCategories, newCategories.constFirst());
    }

    // Sync 2 [out]
    syncer2->setNoPack(true);
    syncSpy2.clear();
    syncer2->setSyncPath(QUrl::fromLocalFile(syncDir2->path()));
    syncer2->sync(syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in-out]
    syncSpy1.clear();
    syncer1->setSyncPath(QUrl::fromLocalFile(syncDir2->path()));
    syncer1->sync(syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    QDateTime maxPackedDate = monthStart(syncStart).addMSecs(-1);
    QVector<QDateTime> unpacked;
    if (!newEntries.isEmpty()) {
        unpacked.append(newEntries.constFirst().sync.mTime);
    }
    if (!newCategories.isEmpty()) {
        unpacked.append(newCategories.constFirst().sync.mTime);
    }

    checkFunction(checkDB, history1, updatedEntries);
    checkFunction(checkDB, history1, updatedCategories);
    checkFunction(checkDB, history1, updatedSyncEntries, updatedSyncCategories);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, maxPackedDate, unpacked);

    // Sync 2 [in]
    syncer2->setNoPack(false);
    syncSpy2.clear();
    syncer2->setSyncPath(QUrl::fromLocalFile(syncDir2->path()));
    syncer2->sync(syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, updatedEntries);
    checkFunction(checkDB, history2, updatedCategories);
    checkFunction(checkDB, history2, updatedSyncEntries, updatedSyncCategories);
    checkFunction(checkPackFolder,
                  QDir(dataDir2->path()).filePath("sync"),
                  maxPackedDate < unpacked.constFirst() ? origSyncEntries : updatedSyncEntries,
                  maxPackedDate < unpacked.constFirst() ? origSyncCategories : updatedSyncCategories,
                  syncPortion,
                  maxPackedDate,
                  maxPackedDate < unpacked.constFirst() ? unpacked : QVector<QDateTime>());

    // Sync 3 [in]
    syncer3->setSyncPath(QUrl::fromLocalFile(syncDir2->path()));
    syncer3->sync(syncStart);
    QVERIFY(syncSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history3, updatedEntries);
    checkFunction(checkDB, history3, updatedCategories);
    checkFunction(checkDB, history3, updatedSyncEntries, updatedSyncCategories);
    checkFunction(checkPackFolder,
                  QDir(dataDir2->path()).filePath("sync"),
                  maxPackedDate < unpacked.constFirst() ? origSyncEntries : updatedSyncEntries,
                  maxPackedDate < unpacked.constFirst() ? origSyncCategories : updatedSyncCategories,
                  syncPortion,
                  maxPackedDate,
                  maxPackedDate < unpacked.constFirst() ? unpacked : QVector<QDateTime>());
}

void tst_SyncPack::unpacked_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");
    QTest::addColumn<int>("syncPortion");
    QTest::addColumn<QDateTime>("syncStart");

    QTest::addColumn<QVector<TimeLogSyncDataEntry> >("newEntries");
    QTest::addColumn<QVector<TimeLogSyncDataCategory> >("newCategories");

    auto addInsertTest = [](int size, int syncPortion, int packCount, int index,
                            const QDateTime &syncStart, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.startTime = defaultEntries().at(index).startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        entry.uuid = QUuid::createUuid();
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, add %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << 0 << syncPortion << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category;
        category.name = "CategoryNew";
        category.uuid = QUuid::createUuid();
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories by %2, pack %3, add %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << 0 << size << syncPortion << syncStart
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories by %2, pack %3, add %4 %5").arg(size)
                      .arg(syncPortion * 2).arg(packCount * 2).arg(index).arg(info).toLocal8Bit())
                << size << size << (syncPortion * 2) << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addInsertTests = [&addInsertTest](int size, int syncPortion, int packCount, int index, const QDateTime &syncStart)
    {
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart), "0");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1), "-1 ms");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1), "+1 ms");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-999), "-999 ms");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(999), "+999 ms");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(-1), "-1 s");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(1), "+1 s");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1001), "-1001 ms");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1001), "+1001 ms");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addDays(-10), "-10 days");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addDays(10), "+10 days");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(-1), "-1 months");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(1), "+1 months");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(-2), "-2 months");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(2), "+2 months");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addYears(-1), "-1 year");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addYears(1), "+1 year");
    };

    addInsertTests(1, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addInsertTests(2, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addInsertTests(2, 2, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addInsertTests(2, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addInsertTests(4, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addInsertTests(4, 1, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(4, 2, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addInsertTests(6, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addInsertTests(6, 1, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(6, 2, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(6, 3, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(6, 5, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(6, 6, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    auto addEditTest = [](int size, int syncPortion, int packCount, int index,
                          const QDateTime &syncStart, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry = defaultEntries().at(index);
        entry.startTime = entry.startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, edit %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << 0 << syncPortion << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category = defaultCategories().at(index);
        category.name = "CategoryNew";
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories by %2, pack %3, edit %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << 0 << size << syncPortion << syncStart
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories by %2, pack %3, edit %4 %5").arg(size)
                      .arg(syncPortion * 2).arg(packCount * 2).arg(index).arg(info).toLocal8Bit())
                << size << size << (syncPortion * 2) << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addEditTests = [&addEditTest](int size, int syncPortion, int packCount, int index, const QDateTime &syncStart)
    {
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart), "0");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1), "-1 ms");
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1), "+1 ms");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-999), "-999 ms");
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(999), "+999 ms");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(-1), "-1 s");
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(1), "+1 s");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1001), "-1001 ms");
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1001), "+1001 ms");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(1), "+1 months");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(2), "+2 months");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addYears(1), "+1 year");
    };

    addEditTests(1, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addEditTests(2, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addEditTests(2, 2, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addEditTests(2, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addEditTests(4, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addEditTests(4, 1, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(4, 2, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addEditTests(6, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addEditTests(6, 1, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(6, 2, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(6, 3, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(6, 5, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(6, 6, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    auto addRemoveTest = [](int size, int syncPortion, int packCount, int index,
                            const QDateTime &syncStart, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.uuid = defaultEntries().at(index).uuid;
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, remove %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << 0 << syncPortion << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category;
        category.uuid = defaultCategories().at(index).uuid;
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories by %2, pack %3, remove %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << 0 << size << syncPortion << syncStart
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories by %2, pack %3, remove %4 %5").arg(size)
                      .arg(syncPortion * 2).arg(packCount * 2).arg(index).arg(info).toLocal8Bit())
                << size << size << (syncPortion * 2) << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addRemoveTests = [&addRemoveTest](int size, int syncPortion, int packCount, int index, const QDateTime &syncStart)
    {
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart), "0");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1), "-1 ms");
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1), "+1 ms");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-999), "-999 ms");
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(999), "+999 ms");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(-1), "-1 s");
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(1), "+1 s");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1001), "-1001 ms");
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1001), "+1001 ms");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(1), "+1 months");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(2), "+2 months");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addYears(1), "+1 year");
    };

    addRemoveTests(1, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addRemoveTests(2, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addRemoveTests(2, 2, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addRemoveTests(2, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addRemoveTests(4, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addRemoveTests(4, 1, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(4, 2, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addRemoveTests(6, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 1, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 2, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 3, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 5, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 6, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
}

void tst_SyncPack::twoPacks()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));
    QSignalSpy syncSpy3(syncer3, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy3(syncer3, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy3(history3, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy3(history3, SIGNAL(dataOutdated()));

    QFETCH(int, syncPortion);
    QFETCH(QDateTime, syncStart);

    checkFunction(importSyncData, history1, syncer1, syncDir1, origSyncEntries, origSyncCategories, syncPortion, false, syncStart);

    QFETCH(QVector<TimeLogSyncDataEntry>, newEntries);
    QFETCH(QVector<TimeLogSyncDataCategory>, newCategories);

    checkFunction(importSyncData, history2, newEntries, newCategories, syncPortion);

    QDateTime newDataMTime;
    if (!newEntries.isEmpty()) {
        updateDataSet(origEntries, newEntries.constFirst().entry);
        updateDataSet(origSyncEntries, newEntries.constFirst());
        newDataMTime = qMax(newDataMTime, newEntries.constFirst().sync.mTime);
    }
    if (!newCategories.isEmpty()) {
        updateDataSet(origCategories, newCategories.constFirst().category);
        updateDataSet(origSyncCategories, newCategories.constFirst());
        newDataMTime = qMax(newDataMTime, newCategories.constFirst().sync.mTime);
    }

    // Sync 2 [out]
    syncer2->setNoPack(false);
    syncSpy2.clear();
    syncer2->setSyncPath(QUrl::fromLocalFile(syncDir2->path()));
    syncer2->sync(monthStart(newDataMTime.addMonths(1)));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in-out]
    syncSpy1.clear();
    syncer1->setSyncPath(QUrl::fromLocalFile(syncDir2->path()));
    syncer1->sync(syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    QDateTime maxPackedDate = qMax(monthStart(syncStart).addMSecs(-1),
                                   monthStart(newDataMTime.addMonths(1)).addMSecs(-1));

    checkFunction(checkDB, history1, origEntries);
    checkFunction(checkDB, history1, origCategories);
    checkFunction(checkDB, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, maxPackedDate);

    // Sync 2 [in]
    syncSpy2.clear();
    syncer2->setSyncPath(QUrl::fromLocalFile(syncDir2->path()));
    syncer2->sync(syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, origEntries);
    checkFunction(checkDB, history2, origCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
    checkFunction(checkPackFolder, QDir(dataDir2->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, maxPackedDate);

    // Sync 3 [in]
    syncer3->setSyncPath(QUrl::fromLocalFile(syncDir2->path()));
    syncer3->sync(syncStart);
    QVERIFY(syncSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history3, origEntries);
    checkFunction(checkDB, history3, origCategories);
    checkFunction(checkDB, history3, origSyncEntries, origSyncCategories);
    checkFunction(checkPackFolder, QDir(dataDir2->path()).filePath("sync"), origSyncEntries,
                  origSyncCategories, syncPortion, maxPackedDate);
}

void tst_SyncPack::twoPacks_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");
    QTest::addColumn<int>("syncPortion");
    QTest::addColumn<QDateTime>("syncStart");

    QTest::addColumn<QVector<TimeLogSyncDataEntry> >("newEntries");
    QTest::addColumn<QVector<TimeLogSyncDataCategory> >("newCategories");

    auto addInsertTest = [](int size, int syncPortion, int packCount, int index,
                            const QDateTime &syncStart, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.startTime = defaultEntries().at(index).startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        entry.uuid = QUuid::createUuid();
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, add %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << 0 << syncPortion << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category;
        category.name = "CategoryNew";
        category.uuid = QUuid::createUuid();
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories by %2, pack %3, add %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << 0 << size << syncPortion << syncStart
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories by %2, pack %3, add %4 %5").arg(size)
                      .arg(syncPortion * 2).arg(packCount * 2).arg(index).arg(info).toLocal8Bit())
                << size << size << (syncPortion * 2) << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addInsertTests = [&addInsertTest](int size, int syncPortion, int packCount, int index, const QDateTime &syncStart)
    {
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart), "0");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1), "-1 ms");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1), "+1 ms");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-999), "-999 ms");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(999), "+999 ms");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(-1), "-1 s");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(1), "+1 s");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1001), "-1001 ms");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1001), "+1001 ms");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addDays(-10), "-10 days");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addDays(10), "+10 days");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(-1), "-1 month");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(1), "+1 month");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(-2), "-2 months");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(2), "+2 months");

        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addYears(-1), "-1 year");
        addInsertTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addYears(1), "+1 year");
    };

    addInsertTests(1, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addInsertTests(2, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addInsertTests(2, 2, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addInsertTests(2, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addInsertTests(4, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addInsertTests(4, 1, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(4, 2, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addInsertTests(6, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addInsertTests(6, 1, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(6, 2, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(6, 3, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(6, 5, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addInsertTests(6, 6, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    auto addEditTest = [](int size, int syncPortion, int packCount, int index,
                          const QDateTime &syncStart, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry = defaultEntries().at(index);
        entry.startTime = entry.startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, edit %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << 0 << syncPortion << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category = defaultCategories().at(index);
        category.name = "CategoryNew";
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories by %2, pack %3, edit %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << 0 << size << syncPortion << syncStart
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories by %2, pack %3, edit %4 %5").arg(size)
                      .arg(syncPortion * 2).arg(packCount * 2).arg(index).arg(info).toLocal8Bit())
                << size << size << (syncPortion * 2) << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addEditTests = [&addEditTest](int size, int syncPortion, int packCount, int index, const QDateTime &syncStart)
    {
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart), "0");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1), "-1 ms");
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1), "+1 ms");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-999), "-999 ms");
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(999), "+999 ms");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(-1), "-1 s");
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(1), "+1 s");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1001), "-1001 ms");
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1001), "+1001 ms");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addDays(-10), "-10 days");
        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addDays(10), "+10 days");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(1), "+1 month");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(2), "+2 months");

        addEditTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addYears(1), "+1 year");
    };

    addEditTests(1, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addEditTests(2, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addEditTests(2, 2, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addEditTests(2, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addEditTests(4, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addEditTests(4, 1, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(4, 2, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addEditTests(6, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addEditTests(6, 1, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(6, 2, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(6, 3, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(6, 5, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addEditTests(6, 6, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    auto addRemoveTest = [](int size, int syncPortion, int packCount, int index,
                            const QDateTime &syncStart, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.uuid = defaultEntries().at(index).uuid;
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, remove %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << 0 << syncPortion << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category;
        category.uuid = defaultCategories().at(index).uuid;
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories by %2, pack %3, remove %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << 0 << size << syncPortion << syncStart
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories by %2, pack %3, remove %4 %5").arg(size)
                      .arg(syncPortion * 2).arg(packCount * 2).arg(index).arg(info).toLocal8Bit())
                << size << size << (syncPortion * 2) << syncStart
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addRemoveTests = [&addRemoveTest](int size, int syncPortion, int packCount, int index, const QDateTime &syncStart)
    {
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart), "0");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1), "-1 ms");
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1), "+1 ms");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-999), "-999 ms");
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(999), "+999 ms");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(-1), "-1 s");
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addSecs(1), "+1 s");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(-1001), "-1001 ms");
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMSecs(1001), "+1001 ms");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addDays(-10), "-10 days");
        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addDays(10), "+10 days");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(1), "+1 month");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addMonths(2), "+2 months");

        addRemoveTest(size, syncPortion, packCount, index, syncStart, monthStart(syncStart).addYears(1), "+1 year");
    };

    addRemoveTests(1, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addRemoveTests(2, 1, 1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addRemoveTests(2, 2, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addRemoveTests(2, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addRemoveTests(4, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addRemoveTests(4, 1, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(4, 2, 4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addRemoveTests(6, 1, 2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 1, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 2, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 3, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 5, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
    addRemoveTests(6, 6, 6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
}

QTEST_MAIN(tst_SyncPack)

#include "tst_sync_pack.moc"
