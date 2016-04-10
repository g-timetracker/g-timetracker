#include <QtTest/QtTest>

#include <QTemporaryDir>

#include "tst_common.h"
#include "DBSyncer.h"
#include "TimeLogCategoryTreeNode.h"

QTemporaryDir *dataDir1 = nullptr;
QTemporaryDir *dataDir2 = nullptr;
TimeLogHistory *history1 = nullptr;
TimeLogHistory *history2 = nullptr;
DBSyncer *dbSyncer = nullptr;

class tst_DBSyncer : public QObject
{
    Q_OBJECT

public:
    tst_DBSyncer();

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void import();
    void import_data();
    void updateHashes();
    void updateHashes_data();
    void bothChange();
    void bothChange_data();
};

tst_DBSyncer::tst_DBSyncer()
{
}

void tst_DBSyncer::init()
{
    dataDir1 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir1);
    QVERIFY(dataDir1->isValid());
    history1 = new TimeLogHistory;
    Q_CHECK_PTR(history1);
    QVERIFY(history1->init(dataDir1->path()));

    dataDir2 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir2);
    QVERIFY(dataDir2->isValid());
    history2 = new TimeLogHistory;
    Q_CHECK_PTR(history2);
    QVERIFY(history2->init(dataDir2->path()));

    dbSyncer = new DBSyncer(history1, history2);
    Q_CHECK_PTR(dbSyncer);
}

void tst_DBSyncer::cleanup()
{
    if (QTest::currentTestFailed()) {
//        dataDir1->setAutoRemove(false);
//        dataDir2->setAutoRemove(false);
//        syncDir->setAutoRemove(false);
    }

    delete dbSyncer;
    dbSyncer = nullptr;

    delete history1;
    history1 = nullptr;
    delete dataDir1;
    dataDir1 = nullptr;

    delete history2;
    history2 = nullptr;
    delete dataDir2;
    dataDir2 = nullptr;
}

void tst_DBSyncer::initTestCase()
{
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();
    qRegisterMetaType<QVector<TimeLogSyncDataEntry> >();
    qRegisterMetaType<QVector<TimeLogSyncDataCategory> >();
    qRegisterMetaType<QSharedPointer<TimeLogCategoryTreeNode> >();
    qRegisterMetaType<QMap<QDateTime,QByteArray> >();

    qSetMessagePattern("[%{time}] <%{category}> %{type} (%{file}:%{line}, %{function}) %{message}");
}

void tst_DBSyncer::cleanupTestCase()
{
}

void tst_DBSyncer::import()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy syncSpy(dbSyncer, SIGNAL(finished(QDateTime)));
    QSignalSpy syncErrorSpy(dbSyncer, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    checkFunction(importSyncData, history1, origSyncEntries, origSyncCategories, entriesCount + categoriesCount);

    QFETCH(int, syncSize);
    Q_ASSERT(syncSize <= entriesCount + categoriesCount);
    QFETCH(QDateTime, maxMonth);

    if (maxMonth.isValid()) {
        dbSyncer->start(false, maxMonth);
    } else {
        dbSyncer->start(false);
    }
    QVERIFY(syncSpy.wait());
    QVERIFY(syncErrorSpy.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history1, origEntries);
    checkFunction(checkDB, history1, origCategories);
    checkFunction(checkDB, history1, origSyncEntries, origSyncCategories);

    int syncEntriesSize = (categoriesCount ? syncSize / 2 : syncSize);
    if (syncEntriesSize < entriesCount) {
        origEntries.resize(syncEntriesSize);
        origSyncEntries.resize(syncEntriesSize);
    }
    int syncCategoriesSize = (entriesCount ? syncSize / 2 : syncSize);
    if (syncCategoriesSize < categoriesCount) {
        origCategories.resize(syncCategoriesSize);
        origSyncCategories.resize(syncCategoriesSize);
    }

    checkFunction(checkDB, history2, origEntries);
    checkFunction(checkDB, history2, origCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_DBSyncer::import_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");
    QTest::addColumn<int>("syncSize");
    QTest::addColumn<QDateTime>("maxMonth");

    QTest::newRow("1 entry, 0") << 1 << 0 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("1 entry, 1") << 1 << 0 << 1 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("1 entry, all") << 1 << 0 << 1 << QDateTime();
    QTest::newRow("2 entries, 0") << 2 << 0 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("2 entries, 2") << 2 << 0 << 2 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("2 entries, all") << 2 << 0 << 2 << QDateTime();
    QTest::newRow("4 entries, 0") << 4 << 0 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("4 entries, 2") << 4 << 0 << 2 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("4 entries, 4") << 4 << 0 << 4 << QDateTime(QDate(2015, 12, 1), QTime(), Qt::UTC);
    QTest::newRow("4 entries, all") << 4 << 0 << 4 << QDateTime();
    QTest::newRow("6 entries, 0") << 6 << 0 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("6 entries, 2") << 6 << 0 << 2 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("6 entries, 6") << 6 << 0 << 6 << QDateTime(QDate(2015, 12, 1), QTime(), Qt::UTC);
    QTest::newRow("6 entries, all") << 6 << 0 << 6 << QDateTime();

    QTest::newRow("1 category, 0") << 0 << 1 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("1 category, 1") << 0 << 1 << 1 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("1 category, all") << 0 << 1 << 1 << QDateTime();
    QTest::newRow("2 categories, 0") << 0 << 2 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("2 categories, 2") << 0 << 2 << 2 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("2 categories, all") << 0 << 2 << 2 << QDateTime();
    QTest::newRow("4 categories, 0") << 0 << 4 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("4 categories, 2") << 0 << 4 << 2 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("4 categories, 4") << 0 << 4 << 4 << QDateTime(QDate(2015, 12, 1), QTime(), Qt::UTC);
    QTest::newRow("4 categories, all") << 0 << 4 << 4 << QDateTime();
    QTest::newRow("6 categories, 0") << 0 << 6 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("6 categories, 2") << 0 << 6 << 2 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("6 categories, 6") << 0 << 6 << 6 << QDateTime(QDate(2015, 12, 1), QTime(), Qt::UTC);
    QTest::newRow("6 categories, all") << 0 << 6 << 6 << QDateTime();

    QTest::newRow("1 entry, 1 category 0") << 1 << 1 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("1 entry, 1 category 2") << 1 << 1 << 2 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("1 entry, 1 category all") << 1 << 1 << 2 << QDateTime();
    QTest::newRow("2 entries, 2 categories, 0") << 2 << 2 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("2 entries, 2 categories, 4") << 2 << 2 << 4 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("2 entries, 2 categories, all") << 2 << 2 << 4 << QDateTime();
    QTest::newRow("4 entries, 4 categories, 0") << 4 << 4 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("4 entries, 4 categories, 4") << 4 << 4 << 4 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("4 entries, 4 categories, 8") << 4 << 4 << 8 << QDateTime(QDate(2015, 12, 1), QTime(), Qt::UTC);
    QTest::newRow("4 entries, 4 categories, all") << 4 << 4 << 8 << QDateTime();
    QTest::newRow("6 entries, 6 categories, 0") << 6 << 6 << 0 << QDateTime(QDate(2015, 10, 1), QTime(), Qt::UTC);
    QTest::newRow("6 entries, 6 categories, 4") << 6 << 6 << 4 << QDateTime(QDate(2015, 11, 1), QTime(), Qt::UTC);
    QTest::newRow("6 entries, 6 categories, 12") << 6 << 6 << 12 << QDateTime(QDate(2015, 12, 1), QTime(), Qt::UTC);
    QTest::newRow("6 entries, 6 categories, all") << 6 << 6 << 12 << QDateTime();
}

void tst_DBSyncer::updateHashes()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy syncSpy(dbSyncer, SIGNAL(finished(QDateTime)));
    QSignalSpy syncErrorSpy(dbSyncer, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy historyUpdateHashesSpy(history2, SIGNAL(hashesUpdated()));

    checkFunction(importSyncData, history1, origSyncEntries, origSyncCategories, 1);

    QFETCH(bool, updateHashes);

    historyUpdateHashesSpy.clear();
    dbSyncer->start(updateHashes);
    QVERIFY(syncSpy.wait());
    QCOMPARE(historyUpdateHashesSpy.size(), updateHashes ? 1 : 0);
    QVERIFY(syncErrorSpy.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkHashesUpdated, history2, updateHashes);

    checkFunction(checkDB, history1, origEntries);
    checkFunction(checkDB, history1, origCategories);
    checkFunction(checkDB, history1, origSyncEntries, origSyncCategories);

    checkFunction(checkDB, history2, origEntries);
    checkFunction(checkDB, history2, origCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_DBSyncer::updateHashes_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");
    QTest::addColumn<bool>("updateHashes");

    QTest::newRow("1 entry, no update") << 1 << 0 << false;
    QTest::newRow("1 entry, update") << 1 << 0 << true;
    QTest::newRow("2 entries, no update") << 2 << 0 << false;
    QTest::newRow("2 entries, update") << 2 << 0 << true;
    QTest::newRow("4 entries, no update") << 4 << 0 << false;
    QTest::newRow("4 entries, update") << 4 << 0 << true;
    QTest::newRow("6 entries, no update") << 6 << 0 << false;
    QTest::newRow("6 entries, update") << 6 << 0 << true;

    QTest::newRow("1 category, no update") << 0 << 1 << false;
    QTest::newRow("1 category, update") << 0 << 1 << true;
    QTest::newRow("2 categories, no update") << 0 << 2 << false;
    QTest::newRow("2 categories, update") << 0 << 2 << true;
    QTest::newRow("4 categories, no update") << 0 << 4 << false;
    QTest::newRow("4 categories, update") << 0 << 4 << true;
    QTest::newRow("6 categories, no update") << 0 << 6 << false;
    QTest::newRow("6 categories, update") << 0 << 6 << true;

    QTest::newRow("1 entry, 1 category, no update") << 1 << 1 << false;
    QTest::newRow("1 entry, 1 category, update") << 1 << 1 << true;
    QTest::newRow("2 entries, 2 categories, no update") << 2 << 2 << false;
    QTest::newRow("2 entries, 2 categories, update") << 2 << 2 << true;
    QTest::newRow("4 entries, 4 categories, no update") << 4 << 4 << false;
    QTest::newRow("4 entries, 4 categories, update") << 4 << 4 << true;
    QTest::newRow("6 entries, 6 categories, no update") << 6 << 6 << false;
    QTest::newRow("6 entries, 6 categories, update") << 6 << 6 << true;
}

void tst_DBSyncer::bothChange()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy syncSpy(dbSyncer, SIGNAL(finished(QDateTime)));
    QSignalSpy syncErrorSpy(dbSyncer, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    checkFunction(importSyncData, history1, origSyncEntries, origSyncCategories, entriesCount + categoriesCount);
    checkFunction(importSyncData, history2, origSyncEntries, origSyncCategories, entriesCount + categoriesCount);

    QFETCH(QVector<TimeLogSyncDataEntry>, newEntries);
    QFETCH(QVector<TimeLogSyncDataCategory>, newCategories);

    checkFunction(importSyncData, history1, newEntries, newCategories, 1);

    if (!newEntries.isEmpty()) {
        updateDataSet(origEntries, newEntries.constFirst().entry);
        updateDataSet(origSyncEntries, newEntries.constFirst());
    }
    if (!newCategories.isEmpty()) {
        updateDataSet(origCategories, newCategories.constFirst().category);
        updateDataSet(origSyncCategories, newCategories.constFirst());
    }

    QFETCH(QDateTime, maxMonth);

    if (maxMonth.isValid()) {
        dbSyncer->start(false, maxMonth);
    } else {
        dbSyncer->start(false);
    }
    QVERIFY(syncSpy.wait());
    QVERIFY(syncErrorSpy.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history1, origEntries);
    checkFunction(checkDB, history1, origCategories);
    checkFunction(checkDB, history1, origSyncEntries, origSyncCategories);

    checkFunction(checkDB, history2, origEntries);
    checkFunction(checkDB, history2, origCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_DBSyncer::bothChange_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");
    QTest::addColumn<QDateTime>("maxMonth");

    QTest::addColumn<QVector<TimeLogSyncDataEntry> >("newEntries");
    QTest::addColumn<QVector<TimeLogSyncDataCategory> >("newCategories");

    auto addInsertTest = [](int size, int index, const QDateTime &maxMonth, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.startTime = defaultEntries().at(index).startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        entry.uuid = QUuid::createUuid();
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries, insert %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << 0 << maxMonth
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category;
        category.name = "CategoryNew";
        category.uuid = QUuid::createUuid();
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories, add %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << 0 << size << maxMonth
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories, add %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << size << maxMonth
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addInsertTests = [&addInsertTest](int size, int index, const QDateTime &maxMonth)
    {
        addInsertTest(size, index, maxMonth, monthStart(maxMonth), "0");
        addInsertTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(-1), "-1 ms");
        addInsertTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(1), "+1 ms");
        addInsertTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(-999), "-999 ms");
        addInsertTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(999), "+999 ms");
        addInsertTest(size, index, maxMonth, monthStart(maxMonth).addSecs(-1), "-1 s");
        addInsertTest(size, index, maxMonth, monthStart(maxMonth).addSecs(1), "+1 s");
        addInsertTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(-1001), "-1001 ms");
        addInsertTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(1001), "+1001 ms");
        addInsertTest(size, index, maxMonth, monthStart(maxMonth).addDays(10), "+10 days");
        addInsertTest(size, index, QDateTime(), monthStart(maxMonth).addMonths(2), "+2 months");
        addInsertTest(size, index, QDateTime(), monthStart(maxMonth).addYears(1), "+1 year");
    };

    addInsertTests(1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addInsertTests(2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addInsertTests(4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addInsertTests(6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));


    auto addEditTest = [](int size, int index, const QDateTime &maxMonth, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry = defaultEntries().at(index);
        entry.startTime = entry.startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries, edit %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << 0 << maxMonth
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category = defaultCategories().at(index);
        category.name = "CategoryNew";
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories, edit %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << 0 << size << maxMonth
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories, edit %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << size << maxMonth
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addEditTests = [&addEditTest](int size, int index, const QDateTime &maxMonth)
    {
        addEditTest(size, index, maxMonth, monthStart(maxMonth), "0");
        addEditTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(-1), "-1 ms");
        addEditTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(1), "+1 ms");
        addEditTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(-999), "-999 ms");
        addEditTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(999), "+999 ms");
        addEditTest(size, index, maxMonth, monthStart(maxMonth).addSecs(-1), "-1 s");
        addEditTest(size, index, maxMonth, monthStart(maxMonth).addSecs(1), "+1 s");
        addEditTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(-1001), "-1001 ms");
        addEditTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(1001), "+1001 ms");
        addEditTest(size, index, maxMonth, monthStart(maxMonth).addDays(10), "+10 days");
        addEditTest(size, index, QDateTime(), monthStart(maxMonth).addMonths(2), "+2 months");
        addEditTest(size, index, QDateTime(), monthStart(maxMonth).addYears(1), "+1 year");
    };

    addEditTests(1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addEditTests(2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addEditTests(4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addEditTests(6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    auto addRemoveTest = [](int size, int index, const QDateTime &maxMonth, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.uuid = defaultEntries().at(index).uuid;
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries, remove %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << 0 << maxMonth
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category;
        category.uuid = defaultCategories().at(index).uuid;
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories, remove %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << 0 << size << maxMonth
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories, remove %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << size << maxMonth
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addRemoveTests = [&addRemoveTest](int size, int index, const QDateTime &maxMonth)
    {
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth), "0");
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(-1), "-1 ms");
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(1), "+1 ms");
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(-999), "-999 ms");
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(999), "+999 ms");
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth).addSecs(-1), "-1 s");
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth).addSecs(1), "+1 s");
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(-1001), "-1001 ms");
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth).addMSecs(1001), "+1001 ms");
        addRemoveTest(size, index, maxMonth, monthStart(maxMonth).addDays(10), "+10 days");
        addRemoveTest(size, index, QDateTime(), monthStart(maxMonth).addMonths(2), "+2 months");
        addRemoveTest(size, index, QDateTime(), monthStart(maxMonth).addYears(1), "+1 year");
    };

    addRemoveTests(1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addRemoveTests(2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addRemoveTests(4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addRemoveTests(6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
}

QTEST_MAIN(tst_DBSyncer)

#include "tst_db_syncer.moc"
