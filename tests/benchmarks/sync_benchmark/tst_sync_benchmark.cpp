#include <functional>

#include <QtTest/QtTest>

#include "tst_common.h"
#include "DataSyncer.h"
#include "TimeLogCategory.h"

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

QTemporaryDir *dataDir1 = Q_NULLPTR;
QTemporaryDir *dataDir2 = Q_NULLPTR;
QTemporaryDir *syncDir = Q_NULLPTR;
TimeLogHistory *history1 = Q_NULLPTR;
TimeLogHistory *history2 = Q_NULLPTR;
DataSyncer *syncer1 = Q_NULLPTR;
DataSyncer *syncer2 = Q_NULLPTR;

const int maxTimeout = 300000;

class tst_Sync_benchmark : public QObject
{
    Q_OBJECT

public:
    tst_Sync_benchmark();
    virtual ~tst_Sync_benchmark();

private slots:
    void init();
    void cleanup();
    void initTestCase();

    void import();
    void import_data();
    void insert();
    void insert_data();
    void remove();
    void remove_data();
    void edit();
    void edit_data();
    void renameCategory();
    void renameCategory_data();
};

tst_Sync_benchmark::tst_Sync_benchmark()
{
}

tst_Sync_benchmark::~tst_Sync_benchmark()
{
}

void tst_Sync_benchmark::init()
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

    dataDir2 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir2);
    QVERIFY(dataDir2->isValid());
    history2 = new TimeLogHistory;
    Q_CHECK_PTR(history2);
    QVERIFY(history2->init(dataDir2->path()));
    syncer2 = new DataSyncer(history2);
    Q_CHECK_PTR(syncer2);
    syncer2->init(dataDir2->path());

    syncDir = new QTemporaryDir();
    Q_CHECK_PTR(syncDir);
    QVERIFY(syncDir->isValid());
}

void tst_Sync_benchmark::cleanup()
{
    if (QTest::currentTestFailed()) {
//        dataDir1->setAutoRemove(false);
//        dataDir2->setAutoRemove(false);
//        syncDir->setAutoRemove(false);
    }

    delete syncer1;
    syncer1 = Q_NULLPTR;
    delete history1;
    history1 = Q_NULLPTR;
    delete dataDir1;
    dataDir1 = Q_NULLPTR;

    delete syncer2;
    syncer2 = Q_NULLPTR;
    delete history2;
    history2 = Q_NULLPTR;
    delete dataDir2;
    dataDir2 = Q_NULLPTR;

    delete syncDir;
    syncDir = Q_NULLPTR;
}

void tst_Sync_benchmark::initTestCase()
{
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();
    qRegisterMetaType<QVector<TimeLogSyncData> >();
    qRegisterMetaType<QSharedPointer<TimeLogCategory> >();

    oldCategoryFilter = QLoggingCategory::installFilter(Q_NULLPTR);
    QLoggingCategory::installFilter(syncerCategoryFilter);
}

void tst_Sync_benchmark::import()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(genData(entriesCount));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));
    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));
    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history1->import(origData);
    QVERIFY(importSpy.wait(maxTimeout));
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    QBENCHMARK_ONCE {
    // Sync 1 [out]
        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait(maxTimeout));
        QVERIFY(syncErrorSpy1.isEmpty());
        QVERIFY(historyErrorSpy1.isEmpty());
        QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2.wait(maxTimeout));
        QVERIFY(syncErrorSpy2.isEmpty());
        QVERIFY(historyErrorSpy2.isEmpty());
        QVERIFY(historyOutdateSpy2.isEmpty());
    }
}

void tst_Sync_benchmark::import_data()
{
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("empty db") << 0;
    QTest::newRow("1 entry") << 1;
    QTest::newRow("2 entries") << 2;
    QTest::newRow("10 entries") << 10;
    QTest::newRow("100 entries") << 100;
    QTest::newRow("1 000 entries") << 1000;
    QTest::newRow("10 000 entries") << 10000;
    QTest::newRow("50 000 entries") << 50000;
}

void tst_Sync_benchmark::insert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(genData(initialEntries));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy insertSpy1(history1, SIGNAL(dataInserted(TimeLogEntry)));
    QSignalSpy insertSpy2(history2, SIGNAL(dataInserted(TimeLogEntry)));

    if (initialEntries) {
        QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
        history1->import(origData);
        QVERIFY(importSpy.wait(maxTimeout));

        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait(maxTimeout));

        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2. wait(maxTimeout));
    }

    QFETCH(int, entriesCount);

    for (int i = 0; i < entriesCount; i++) {
        TimeLogEntry entry;
        entry.startTime = origData.constLast().startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        entry.uuid = QUuid::createUuid();
        history1->insert(entry);
        origData.append(entry);
    }
    while (insertSpy1.size() < entriesCount) {
        QVERIFY(insertSpy1.wait(maxTimeout));
    }

    QBENCHMARK_ONCE {
        // Sync 1 [out]
        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait(maxTimeout));
        QVERIFY(syncErrorSpy1.isEmpty());
        QVERIFY(historyErrorSpy1.isEmpty());
        QVERIFY(historyOutdateSpy1.isEmpty());

        // Sync 2 [in]
        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2.wait(maxTimeout));
        QVERIFY(syncErrorSpy2.isEmpty());
        QVERIFY(historyErrorSpy2.isEmpty());
        QVERIFY(historyOutdateSpy2.isEmpty());

        while (insertSpy2.size() < entriesCount) {
            QVERIFY(insertSpy2.wait(maxTimeout));
        }
    }
}

void tst_Sync_benchmark::insert_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("1 entry, 1") << 1 << 1;
    QTest::newRow("1 entry, 10") << 1 << 10;
    QTest::newRow("1 entry, 100") << 1 << 100;

    QTest::newRow("10 entries, 1") << 10 << 1;
    QTest::newRow("10 entries, 10") << 10 << 10;
    QTest::newRow("10 entries, 100") << 10 << 100;

    QTest::newRow("100 entries, 1") << 100 << 1;
    QTest::newRow("100 entries, 10") << 100 << 10;
    QTest::newRow("100 entries, 100") << 100 << 100;

    QTest::newRow("1000 entries, 1") << 1000 << 1;
    QTest::newRow("1000 entries, 10") << 1000 << 10;
    QTest::newRow("1000 entries, 100") << 1000 << 100;

    QTest::newRow("10000 entries, 1") << 10000 << 1;
    QTest::newRow("10000 entries, 10") << 10000 << 10;
    QTest::newRow("10000 entries, 100") << 10000 << 100;

    QTest::newRow("50000 entries, 1") << 50000 << 1;
    QTest::newRow("50000 entries, 10") << 50000 << 10;
    QTest::newRow("50000 entries, 100") << 50000 << 100;
}

void tst_Sync_benchmark::remove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(genData(initialEntries));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy removeSpy1(history1, SIGNAL(dataRemoved(TimeLogEntry)));
    QSignalSpy removeSpy2(history2, SIGNAL(dataRemoved(TimeLogEntry)));

    QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history1->import(origData);
    QVERIFY(importSpy.wait(maxTimeout));

    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait(maxTimeout));

    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2. wait(maxTimeout));

    QFETCH(int, entriesCount);

    std::uniform_int_distribution<> indexDistribution(0, origData.size() - 1);
    std::function<int()> randomIndex = std::bind(indexDistribution, std::default_random_engine());

    QVector<int> indices;

    while (indices.size() < entriesCount) {
        int index = randomIndex();
        if (!indices.contains(index)) {
            indices.append(index);
        }
    }

    for (int i = 0; i < indices.size(); i++) {
        history1->remove(origData.at(indices.at(i)));
    }

    while (removeSpy1.size() < entriesCount) {
        QVERIFY(removeSpy1.wait(maxTimeout));
    }

    QBENCHMARK_ONCE {
        // Sync 1 [out]
        syncSpy1.clear();
        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait(maxTimeout));
        QVERIFY(syncErrorSpy1.isEmpty());
        QVERIFY(historyErrorSpy1.isEmpty());
        QVERIFY(historyOutdateSpy1.isEmpty());

        // Sync 2 [in]
        syncSpy2.clear();
        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2.wait(maxTimeout));
        QVERIFY(syncErrorSpy2.isEmpty());
        QVERIFY(historyErrorSpy2.isEmpty());
        QVERIFY(historyOutdateSpy2.isEmpty());

        while (removeSpy2.size() < entriesCount) {
            QVERIFY(removeSpy2.wait(maxTimeout));
        }
    }
}

void tst_Sync_benchmark::remove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("1 entry, 1") << 1 << 1;

    QTest::newRow("10 entries, 1") << 10 << 1;
    QTest::newRow("10 entries, 10") << 10 << 10;

    QTest::newRow("100 entries, 1") << 100 << 1;
    QTest::newRow("100 entries, 10") << 100 << 10;
    QTest::newRow("100 entries, 100") << 100 << 100;

    QTest::newRow("1000 entries, 1") << 1000 << 1;
    QTest::newRow("1000 entries, 10") << 1000 << 10;
    QTest::newRow("1000 entries, 100") << 1000 << 100;

    QTest::newRow("10000 entries, 1") << 10000 << 1;
    QTest::newRow("10000 entries, 10") << 10000 << 10;
    QTest::newRow("10000 entries, 100") << 10000 << 100;

    QTest::newRow("50000 entries, 1") << 50000 << 1;
    QTest::newRow("50000 entries, 10") << 50000 << 10;
    QTest::newRow("50000 entries, 100") << 50000 << 100;
}

void tst_Sync_benchmark::edit()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(genData(initialEntries));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy historyUpdateSpy1(history1, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy historyUpdateSpy2(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history1->import(origData);
    QVERIFY(importSpy.wait(maxTimeout));

    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait(maxTimeout));

    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2. wait(maxTimeout));

    QFETCH(int, entriesCount);

    std::uniform_int_distribution<> indexDistribution(0, origData.size() - 1);
    std::function<int()> randomIndex = std::bind(indexDistribution, std::default_random_engine());

    QVector<int> indices;

    while (indices.size() < entriesCount) {
        int index = randomIndex();
        if (!indices.contains(index)) {
            indices.append(index);
        }
    }

    for (int i = 0; i < indices.size(); i++) {
        TimeLogEntry entry = origData.at(indices.at(i));
        entry.comment = "New Test comment";
        history1->edit(entry, TimeLogHistory::Comment);
    }
    while (historyUpdateSpy1.size() < entriesCount) {
        QVERIFY(historyUpdateSpy1.wait(maxTimeout));
    }

    QBENCHMARK_ONCE {
        // Sync 1 [out]
        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait(maxTimeout));
        QVERIFY(syncErrorSpy1.isEmpty());
        QVERIFY(historyErrorSpy1.isEmpty());
        QVERIFY(historyOutdateSpy1.isEmpty());

        // Sync 2 [in]
        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2.wait(maxTimeout));
        QVERIFY(syncErrorSpy2.isEmpty());
        QVERIFY(historyErrorSpy2.isEmpty());
        QVERIFY(historyOutdateSpy2.isEmpty());

        while (historyUpdateSpy2.size() < entriesCount) {
            QVERIFY(historyUpdateSpy2.wait(maxTimeout));
        }
    }
}

void tst_Sync_benchmark::edit_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("1 entry, 1") << 1 << 1;

    QTest::newRow("10 entries, 1") << 10 << 1;
    QTest::newRow("10 entries, 10") << 10 << 10;

    QTest::newRow("100 entries, 1") << 100 << 1;
    QTest::newRow("100 entries, 10") << 100 << 10;
    QTest::newRow("100 entries, 100") << 100 << 100;

    QTest::newRow("1000 entries, 1") << 1000 << 1;
    QTest::newRow("1000 entries, 10") << 1000 << 10;
    QTest::newRow("1000 entries, 100") << 1000 << 100;

    QTest::newRow("10000 entries, 1") << 10000 << 1;
    QTest::newRow("10000 entries, 10") << 10000 << 10;
    QTest::newRow("10000 entries, 100") << 10000 << 100;

    QTest::newRow("50000 entries, 1") << 50000 << 1;
    QTest::newRow("50000 entries, 10") << 50000 << 10;
    QTest::newRow("50000 entries, 100") << 50000 << 100;
}

void tst_Sync_benchmark::renameCategory()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(genData(initialEntries));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy historyUpdateSpy(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history1->import(origData);
    QVERIFY(importSpy.wait(maxTimeout));

    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait(maxTimeout));

    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2. wait(maxTimeout));

    int index = std::ceil((origData.size() - 1) / 2.0);
    QString category = origData.at(index).category;
    int count = 0;
    foreach (const TimeLogEntry &entry, origData) {
        if (entry.category == category) {
            count++;
        }
    }

    history1->editCategory(origData.at(index).category, "CategoryNew");
    QVERIFY(historyOutdateSpy1.wait(maxTimeout));
    historyOutdateSpy1.clear();

    QBENCHMARK_ONCE {
        // Sync 1 [out]
        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait(maxTimeout));
        QVERIFY(syncErrorSpy1.isEmpty());
        QVERIFY(historyErrorSpy1.isEmpty());
        QVERIFY(historyOutdateSpy1.isEmpty());

        // Sync 2 [in]
        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2.wait(maxTimeout));
        QVERIFY(syncErrorSpy2.isEmpty());
        QVERIFY(historyErrorSpy2.isEmpty());
        QVERIFY(historyOutdateSpy2.isEmpty());

        while (historyUpdateSpy.size() < count) {
            QVERIFY(historyUpdateSpy.wait(maxTimeout));
        }
    }
}

void tst_Sync_benchmark::renameCategory_data()
{
    QTest::addColumn<int>("initialEntries");

    QTest::newRow("1 entry") << 1;
    QTest::newRow("10 entries") << 10;
    QTest::newRow("100 entries") << 100;
    QTest::newRow("1000 entries") << 1000;
    QTest::newRow("10000 entries") << 10000;
    QTest::newRow("50000 entries") << 50000;
}

QTEST_MAIN(tst_Sync_benchmark)
#include "tst_sync_benchmark.moc"
