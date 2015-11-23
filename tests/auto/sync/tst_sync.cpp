#include <QtTest/QtTest>

#include <QTemporaryDir>

#include "TimeLogHistory.h"
#include "DataSyncer.h"
#include "TimeLogSyncData.h"

#define checkFunction(func, ...) do {   \
    func(__VA_ARGS__);                  \
    if (QTest::currentTestFailed()) {   \
        QFAIL("Subtest failed");        \
    }                                   \
} while (0)

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

QVector<TimeLogEntry> defaultData;

class tst_Sync : public QObject
{
    Q_OBJECT
public:
    tst_Sync();
    virtual ~tst_Sync();

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

    void bothRemove();
    void bothRemove_data();
    void bothEdit();
    void bothEdit_data();
    void editRemove();
    void editRemove_data();
    void removeEdit();
    void removeEdit_data();

private:
    void checkInsert(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index);
    void checkRemove(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index);
    void checkEdit(QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, TimeLogHistory::Fields fields, int index);

    void checkDB(TimeLogHistory *history, const QVector<TimeLogEntry> &data) const;

    void initDefaultData();
    void dumpData(const QVector<TimeLogEntry> &data) const;
    bool checkData(const QVector<TimeLogEntry> &data) const;

    bool compareData(const TimeLogEntry &t1, const TimeLogEntry &t2) const;
    bool compareData(const QVector<TimeLogEntry> &d1, const QVector<TimeLogEntry> &d2) const;
};

tst_Sync::tst_Sync()
{
}

tst_Sync::~tst_Sync()
{
}

void tst_Sync::init()
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

void tst_Sync::cleanup()
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

void tst_Sync::initTestCase()
{
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();
    qRegisterMetaType<QVector<TimeLogSyncData> >();

    initDefaultData();

    oldCategoryFilter = QLoggingCategory::installFilter(Q_NULLPTR);
    QLoggingCategory::installFilter(syncerCategoryFilter);
}

void tst_Sync::import()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(defaultData.mid(0, entriesCount));

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
    QVERIFY(importSpy.wait());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QCOMPARE(history1->size(), origData.size());
    QVERIFY(compareData(importSpy.constFirst().at(0).value<QVector<TimeLogEntry> >(), origData));

    checkFunction(checkDB, history1, origData);

    // Sync 1 [out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origData);

    // Sync 2 [in]
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, origData);

    // Sync 1 [in]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origData);
}

void tst_Sync::import_data()
{
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("empty db") << 0;
    QTest::newRow("1 entry") << 1;
    QTest::newRow("2 entries") << 2;
    QTest::newRow("6 entries") << 6;
}

void tst_Sync::insert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

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

    QSignalSpy historyUpdateSpy(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    if (initialEntries) {
        QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
        history1->import(origData);
        QVERIFY(importSpy.wait());

        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait());

        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2. wait());
    }

    QFETCH(int, index);
    QFETCH(TimeLogEntry, newData);

    newData.uuid = QUuid::createUuid();
    insertSpy1.clear();
    history1->insert(newData);
    QVERIFY(insertSpy1.wait());
    origData.insert(index, newData);

    // Sync 1 [out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    insertSpy2.clear();
    historyUpdateSpy.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkInsert, insertSpy2, historyUpdateSpy, origData, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
}

void tst_Sync::insert_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    TimeLogEntry entry;
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    entry.startTime = QDateTime::fromString("2015-11-01T15:00:00+0200", Qt::ISODate);
    QTest::newRow("empty db") << 0 << 0 << entry;

    entry.startTime = QDateTime::fromString("2015-11-01T10:30:00+0200", Qt::ISODate);
    QTest::newRow("1 entry, begin") << 1 << 0 << entry;

    entry.startTime = QDateTime::fromString("2015-11-01T11:30:00+0200", Qt::ISODate);
    QTest::newRow("1 entry, end") << 1 << 1 << entry;

    entry.startTime = QDateTime::fromString("2015-11-01T11:30:00+0200", Qt::ISODate);
    QTest::newRow("2 entries") << 2 << 2 << entry;

    entry.startTime = QDateTime::fromString("2015-11-01T23:30:00+0200", Qt::ISODate);
    QTest::newRow("6 entries") << 6 << 4 << entry;
}

void tst_Sync::remove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

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

    QSignalSpy historyUpdateSpy(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    QSignalSpy historyDataSpy(history1, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    if (initialEntries) {
        QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
        history1->import(origData);
        QVERIFY(importSpy.wait());

        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait());

        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2. wait());
    }

    historyDataSpy.clear();
    id = QDateTime::currentMSecsSinceEpoch();
    history1->getHistoryBetween(id);
    QVERIFY(historyDataSpy.wait());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QCOMPARE(historyDataSpy.constFirst().at(1).toLongLong(), id);
    historyData = historyDataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

    QFETCH(int, index);

    removeSpy1.clear();
    history1->remove(historyData.at(index));
    QVERIFY(removeSpy1.wait());

    // Sync 1 [out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    removeSpy2.clear();
    historyUpdateSpy.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkRemove, removeSpy2, historyUpdateSpy, origData, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
}

void tst_Sync::remove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries, first") << 2 << 0;
    QTest::newRow("2 entries, last") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 3;
}

void tst_Sync::edit()
{
    QVector<TimeLogEntry> origData(defaultData);

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

    QSignalSpy historyDataSpy(history1, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history1->import(origData);
    QVERIFY(importSpy.wait());

    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());

    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2. wait());

    historyDataSpy.clear();
    id = QDateTime::currentMSecsSinceEpoch();
    history1->getHistoryBetween(id);
    QVERIFY(historyDataSpy.wait());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QCOMPARE(historyDataSpy.constFirst().at(1).toLongLong(), id);
    historyData = historyDataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

    QFETCH(int, index);
    QFETCH(TimeLogEntry, newData);

    TimeLogEntry entry = historyData.at(index);
    TimeLogHistory::Fields fields;
    if (entry.startTime != newData.startTime) {
        entry.startTime = newData.startTime;
        fields |= TimeLogHistory::StartTime;
    }
    if (entry.category != newData.category) {
        entry.category = newData.category;
        fields |= TimeLogHistory::Category;
    }
    if (entry.comment != newData.comment) {
        entry.comment = newData.comment;
        fields |= TimeLogHistory::Comment;
    }
    historyUpdateSpy1.clear();
    history1->edit(entry, fields);
    QVERIFY(historyUpdateSpy1.wait());
    origData[index] = entry;

    // Sync 1 [out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    historyUpdateSpy2.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkEdit, historyUpdateSpy2, origData, fields, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
}

void tst_Sync::edit_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultData.at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultData.at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;
}

void tst_Sync::renameCategory()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy historyUpdateSpy(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    QFETCH(QString, categoryOld);
    QFETCH(QString, categoryNew);
    QFETCH(QVector<int>, indexes);
    foreach (int index, indexes) {
        origData[index].category = categoryOld;
    }

    QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history1->import(origData);
    QVERIFY(importSpy.wait());

    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());

    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2. wait());

    history1->editCategory(categoryOld, categoryNew);
    QVERIFY(historyOutdateSpy1.wait());
    QVERIFY(historyErrorSpy1.isEmpty());

    foreach (int index, indexes) {
        origData[index].category = categoryNew;
    }

    // Sync 1 [out]
    historyOutdateSpy1.clear();
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    historyUpdateSpy.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(historyOutdateSpy2.isEmpty());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());

    while (historyUpdateSpy.size() < indexes.size()) {
        QVERIFY(historyUpdateSpy.wait());
    }

    for (int i = 0; i < indexes.size(); i++) {
        QVector<TimeLogEntry> updateData = historyUpdateSpy.at(i).at(0).value<QVector<TimeLogEntry> >();
        QVector<TimeLogHistory::Fields> updateFields = historyUpdateSpy.at(i).at(1).value<QVector<TimeLogHistory::Fields> >();
        QCOMPARE(updateData.size(), 1);
        QCOMPARE(updateData.constFirst().category, origData.at(indexes.at(i)).category);
        QCOMPARE(updateFields.constFirst(), TimeLogHistory::Category);
    }

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
}

void tst_Sync::renameCategory_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<QString>("categoryOld");
    QTest::addColumn<QString>("categoryNew");
    QTest::addColumn<QVector<int> >("indexes");

    QTest::newRow("6 entries, 2 items") << 6 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 1 << 3);
    QTest::newRow("1 entry") << 1 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 0);
    QTest::newRow("2 entries, first") << 2 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 0);
    QTest::newRow("2 entries, last") << 2 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 1);
    QTest::newRow("all items") << 6 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 0 << 1 << 2 << 3 << 4 << 5);
}

void tst_Sync::bothRemove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

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

    QSignalSpy historyUpdateSpy1(history1, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy historyUpdateSpy2(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    QSignalSpy historyDataSpy(history1, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    if (initialEntries) {
        QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
        history1->import(origData);
        QVERIFY(importSpy.wait());

        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait());

        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2. wait());
    }

    historyDataSpy.clear();
    id = QDateTime::currentMSecsSinceEpoch();
    history1->getHistoryBetween(id);
    QVERIFY(historyDataSpy.wait());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QCOMPARE(historyDataSpy.constFirst().at(1).toLongLong(), id);
    historyData = historyDataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

    QFETCH(int, index);

    removeSpy1.clear();
    history1->remove(historyData.at(index));
    QVERIFY(removeSpy1.wait());

    removeSpy2.clear();
    history2->remove(historyData.at(index));
    QVERIFY(removeSpy2.wait());

    // Sync 2 [out]
    removeSpy2.clear();
    historyUpdateSpy2.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in]
    removeSpy1.clear();
    historyUpdateSpy1.clear();
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    QVERIFY(removeSpy1.isEmpty());
    QVERIFY(historyUpdateSpy1.isEmpty());

    origData.remove(index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
}

void tst_Sync::bothRemove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries, first") << 2 << 0;
    QTest::newRow("2 entries, last") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 3;
}

void tst_Sync::bothEdit()
{
    QVector<TimeLogEntry> origData(defaultData);

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

    QSignalSpy historyDataSpy(history1, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history1->import(origData);
    QVERIFY(importSpy.wait());

    // Sync 1 [out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());

    // Sync 2 [in]
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2. wait());

    historyDataSpy.clear();
    id = QDateTime::currentMSecsSinceEpoch();
    history1->getHistoryBetween(id);
    QVERIFY(historyDataSpy.wait());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QCOMPARE(historyDataSpy.constFirst().at(1).toLongLong(), id);
    historyData = historyDataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

    QFETCH(int, index);
    QFETCH(TimeLogEntry, newData1);
    QFETCH(TimeLogEntry, newData2);

    TimeLogEntry entry = historyData.at(index);
    TimeLogHistory::Fields fields;
    if (entry.startTime != newData1.startTime) {
        entry.startTime = newData1.startTime;
        fields |= TimeLogHistory::StartTime;
    }
    if (entry.category != newData1.category) {
        entry.category = newData1.category;
        fields |= TimeLogHistory::Category;
    }
    if (entry.comment != newData1.comment) {
        entry.comment = newData1.comment;
        fields |= TimeLogHistory::Comment;
    }
    historyUpdateSpy1.clear();
    history1->edit(entry, fields);
    QVERIFY(historyUpdateSpy1.wait());

    entry = historyData.at(index);
    fields = TimeLogHistory::NoFields;
    if (entry.startTime != newData2.startTime) {
        entry.startTime = newData2.startTime;
        fields |= TimeLogHistory::StartTime;
    }
    if (entry.category != newData2.category) {
        entry.category = newData2.category;
        fields |= TimeLogHistory::Category;
    }
    if (entry.comment != newData2.comment) {
        entry.comment = newData2.comment;
        fields |= TimeLogHistory::Comment;
    }
    historyUpdateSpy2.clear();
    history2->edit(entry, fields);
    QVERIFY(historyUpdateSpy2.wait());
    origData[index] = entry;

    // Sync 2 [out]
    historyUpdateSpy2.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in]
    historyUpdateSpy1.clear();
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkEdit, historyUpdateSpy1, origData, fields, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
}

void tst_Sync::bothEdit_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData1");
    QTest::addColumn<TimeLogEntry>("newData2");

    int index;
    TimeLogEntry entry1;
    TimeLogEntry entry2;

    index = 4;
    entry1 = defaultData.at(index);
    entry1.category = "CategoryOld";
    entry2 = defaultData.at(index);
    entry2.category = "CategoryNew";
    QTest::newRow("category") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultData.at(index);
    entry1.comment = "Test comment old";
    entry2 = defaultData.at(index);
    entry2.comment = "Test comment new";
    QTest::newRow("comment") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultData.at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry2 = defaultData.at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    QTest::newRow("start") << index << entry1 << entry2;

    index = 3;
    entry1 = defaultData.at(index);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultData.at(index);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("category & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultData.at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry2 = defaultData.at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultData.at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry1.comment = "Test comment iold";
    entry2 = defaultData.at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    entry2.comment = "Test comment new";
    QTest::newRow("start & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultData.at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultData.at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("all") << index << entry1 << entry2;
}

void tst_Sync::editRemove()
{
    QVector<TimeLogEntry> origData(defaultData);

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

    QSignalSpy historyUpdateSpy(history1, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    QSignalSpy historyDataSpy(history1, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history1->import(origData);
    QVERIFY(importSpy.wait());

    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());

    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2. wait());

    historyDataSpy.clear();
    id = QDateTime::currentMSecsSinceEpoch();
    history1->getHistoryBetween(id);
    QVERIFY(historyDataSpy.wait());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QCOMPARE(historyDataSpy.constFirst().at(1).toLongLong(), id);
    historyData = historyDataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

    QFETCH(int, index);
    QFETCH(TimeLogEntry, newData);

    TimeLogEntry entry = historyData.at(index);
    TimeLogHistory::Fields fields;
    if (entry.startTime != newData.startTime) {
        entry.startTime = newData.startTime;
        fields |= TimeLogHistory::StartTime;
    }
    if (entry.category != newData.category) {
        entry.category = newData.category;
        fields |= TimeLogHistory::Category;
    }
    if (entry.comment != newData.comment) {
        entry.comment = newData.comment;
        fields |= TimeLogHistory::Comment;
    }
    historyUpdateSpy.clear();
    history1->edit(entry, fields);
    QVERIFY(historyUpdateSpy.wait());

    removeSpy2.clear();
    history2->remove(historyData.at(index));
    QVERIFY(removeSpy2.wait());

    // Sync 2 [out]
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in]
    removeSpy1.clear();
    historyUpdateSpy.clear();
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkRemove, removeSpy1, historyUpdateSpy, origData, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
}

void tst_Sync::editRemove_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultData.at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultData.at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;
}

void tst_Sync::removeEdit()
{
    QVector<TimeLogEntry> origData(defaultData);

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy insertSpy(history1, SIGNAL(dataInserted(TimeLogEntry)));

    QSignalSpy removeSpy(history1, SIGNAL(dataRemoved(TimeLogEntry)));

    QSignalSpy historyUpdateSpy1(history1, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy historyUpdateSpy2(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    QSignalSpy historyDataSpy(history1, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history1->import(origData);
    QVERIFY(importSpy.wait());

    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());

    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2. wait());

    historyDataSpy.clear();
    id = QDateTime::currentMSecsSinceEpoch();
    history1->getHistoryBetween(id);
    QVERIFY(historyDataSpy.wait());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());
    QCOMPARE(historyDataSpy.constFirst().at(1).toLongLong(), id);
    historyData = historyDataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

    QFETCH(int, index);
    QFETCH(TimeLogEntry, newData);

    removeSpy.clear();
    history1->remove(historyData.at(index));
    QVERIFY(removeSpy.wait());

    TimeLogEntry entry = historyData.at(index);
    TimeLogHistory::Fields fields;
    if (entry.startTime != newData.startTime) {
        entry.startTime = newData.startTime;
        fields |= TimeLogHistory::StartTime;
    }
    if (entry.category != newData.category) {
        entry.category = newData.category;
        fields |= TimeLogHistory::Category;
    }
    if (entry.comment != newData.comment) {
        entry.comment = newData.comment;
        fields |= TimeLogHistory::Comment;
    }
    historyUpdateSpy2.clear();
    history2->edit(entry, fields);
    QVERIFY(historyUpdateSpy2.wait());
    origData[index] = entry;

    // Sync 2 [out]
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in]
    insertSpy.clear();
    removeSpy.clear();
    historyUpdateSpy1.clear();
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    QVERIFY(removeSpy.isEmpty());
    checkFunction(checkInsert, insertSpy, historyUpdateSpy1, origData, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
}

void tst_Sync::removeEdit_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultData.at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultData.at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;
}

void tst_Sync::checkInsert(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index)
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

void tst_Sync::checkRemove(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index)
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

void tst_Sync::checkEdit(QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, TimeLogHistory::Fields fields, int index)
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

void tst_Sync::checkDB(TimeLogHistory *history, const QVector<TimeLogEntry> &data) const
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

void tst_Sync::initDefaultData()
{
    defaultData.append(TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T11:00:00+0200",
                                                                                           Qt::ISODate),
                                                                     "Category0", "")));
    defaultData.append(TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T11:00:01+0200",
                                                                                           Qt::ISODate),
                                                                     "Category1", "")));
    defaultData.append(TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T11:59:59+0200",
                                                                                           Qt::ISODate),
                                                                     "Category2", "")));
    defaultData.append(TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T12:00:00+0200",
                                                                                           Qt::ISODate),
                                                                     "Category3", "")));
    defaultData.append(TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-01T23:59:59+0200",
                                                                                           Qt::ISODate),
                                                                     "Category4", "")));
    defaultData.append(TimeLogEntry(QUuid::createUuid(), TimeLogData(QDateTime::fromString("2015-11-02T00:00:00+0200",
                                                                                           Qt::ISODate),
                                                                     "Category5", "")));
}

void tst_Sync::dumpData(const QVector<TimeLogEntry> &data) const
{
    for (int i = 0; i < data.size(); i++) {
        qDebug() << i << data.at(i) << endl;
    }
}

bool tst_Sync::checkData(const QVector<TimeLogEntry> &data) const
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

bool tst_Sync::compareData(const TimeLogEntry &t1, const TimeLogEntry &t2) const
{
    if (t1.uuid != t2.uuid || t1.startTime != t2.startTime || t1.category != t2.category
        || t1.comment != t2.comment) {
        qCritical() << "Data does not match" << endl << t1 << endl << t2 << endl;
        return false;
    }

    return true;
}

bool tst_Sync::compareData(const QVector<TimeLogEntry> &d1, const QVector<TimeLogEntry> &d2) const
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

QTEST_MAIN(tst_Sync)
#include "tst_sync.moc"
