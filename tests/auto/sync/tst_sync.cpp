#include <QtTest/QtTest>

#include <QTemporaryDir>

#include "tst_common.h"
#include "DataSyncer.h"

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
QTemporaryDir *dataDir3 = Q_NULLPTR;
QTemporaryDir *syncDir = Q_NULLPTR;
TimeLogHistory *history1 = Q_NULLPTR;
TimeLogHistory *history2 = Q_NULLPTR;
TimeLogHistory *history3 = Q_NULLPTR;
DataSyncer *syncer1 = Q_NULLPTR;
DataSyncer *syncer2 = Q_NULLPTR;
DataSyncer *syncer3 = Q_NULLPTR;

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

    void removeOldEdit();
    void removeOldEdit_data();
    void removeOldInsert();
    void removeOldInsert_data();
    void removeOldRemove();
    void removeOldRemove_data();
    void editOldEdit();
    void editOldEdit_data();
    void editOldRemove();
    void editOldRemove_data();
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

    dataDir3 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir3);
    QVERIFY(dataDir3->isValid());
    history3 = new TimeLogHistory;
    Q_CHECK_PTR(history3);
    QVERIFY(history3->init(dataDir3->path()));
    syncer3 = new DataSyncer(history3);
    Q_CHECK_PTR(syncer3);
    syncer3->init(dataDir3->path());

    syncDir = new QTemporaryDir();
    Q_CHECK_PTR(syncDir);
    QVERIFY(syncDir->isValid());
}

void tst_Sync::cleanup()
{
    if (QTest::currentTestFailed()) {
//        dataDir1->setAutoRemove(false);
//        dataDir2->setAutoRemove(false);
//        dataDir3->setAutoRemove(false);
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

    delete syncer3;
    syncer3 = Q_NULLPTR;
    delete history3;
    history3 = Q_NULLPTR;
    delete dataDir3;
    dataDir3 = Q_NULLPTR;

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

    oldCategoryFilter = QLoggingCategory::installFilter(Q_NULLPTR);
    QLoggingCategory::installFilter(syncerCategoryFilter);
}

void tst_Sync::import()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(defaultData().mid(0, entriesCount));

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

    QVector<TimeLogEntry> origData(defaultData().mid(0, initialEntries));

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

    QVector<TimeLogEntry> origData(defaultData().mid(0, initialEntries));

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
    QVector<TimeLogEntry> origData(defaultData());

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
    TimeLogEntry entry = defaultData().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultData().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;
}

void tst_Sync::renameCategory()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData().mid(0, initialEntries));

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

    QVector<TimeLogEntry> origData(defaultData().mid(0, initialEntries));

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
    QVector<TimeLogEntry> origData(defaultData());

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
    entry1 = defaultData().at(index);
    entry1.category = "CategoryOld";
    entry2 = defaultData().at(index);
    entry2.category = "CategoryNew";
    QTest::newRow("category") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultData().at(index);
    entry1.comment = "Test comment old";
    entry2 = defaultData().at(index);
    entry2.comment = "Test comment new";
    QTest::newRow("comment") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultData().at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry2 = defaultData().at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    QTest::newRow("start") << index << entry1 << entry2;

    index = 3;
    entry1 = defaultData().at(index);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultData().at(index);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("category & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultData().at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry2 = defaultData().at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultData().at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry1.comment = "Test comment iold";
    entry2 = defaultData().at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    entry2.comment = "Test comment new";
    QTest::newRow("start & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultData().at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultData().at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("all") << index << entry1 << entry2;
}

void tst_Sync::editRemove()
{
    QVector<TimeLogEntry> origData(defaultData());

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
    TimeLogEntry entry = defaultData().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultData().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;
}

void tst_Sync::removeEdit()
{
    QVector<TimeLogEntry> origData(defaultData());

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
    TimeLogEntry entry = defaultData().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultData().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;
}

void tst_Sync::removeOldEdit()
{
    QVector<TimeLogEntry> origData(defaultData());

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

    QSignalSpy removeSpy2(history2, SIGNAL(dataRemoved(TimeLogEntry)));
    QSignalSpy removeSpy3(history3, SIGNAL(dataRemoved(TimeLogEntry)));

    QSignalSpy historyUpdateSpy1(history1, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy historyUpdateSpy3(history3, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

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

    // Sync 3 [in]
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3. wait());

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

    removeSpy2.clear();
    history2->remove(historyData.at(index));
    QVERIFY(removeSpy2.wait());

    // Sync 2 [out]
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 3 [in]
    removeSpy3.clear();
    historyUpdateSpy3.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(!removeSpy3.isEmpty() || removeSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkRemove, removeSpy3, historyUpdateSpy3, origData, index);

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy3.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(historyUpdateSpy3.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);
}

void tst_Sync::removeOldEdit_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultData().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultData().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;
}

void tst_Sync::removeOldInsert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData().mid(0, initialEntries));

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

    QSignalSpy insertSpy1(history1, SIGNAL(dataInserted(TimeLogEntry)));
    QSignalSpy insertSpy2(history2, SIGNAL(dataInserted(TimeLogEntry)));
    QSignalSpy insertSpy3(history3, SIGNAL(dataInserted(TimeLogEntry)));

    QSignalSpy removeSpy(history2, SIGNAL(dataRemoved(TimeLogEntry)));

    QSignalSpy historyUpdateSpy(history3, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    if (initialEntries) {
        QSignalSpy importSpy(history1, SIGNAL(dataImported(QVector<TimeLogEntry>)));
        history1->import(origData);
        QVERIFY(importSpy.wait());

        syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy1.wait());

        syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy2. wait());

        syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
        QVERIFY(syncSpy3. wait());
    }

    QFETCH(TimeLogEntry, newData);

    newData.uuid = QUuid::createUuid();
    insertSpy1.clear();
    history1->insert(newData);
    QVERIFY(insertSpy1.wait());

    insertSpy2.clear();
    history2->insert(newData);
    QVERIFY(insertSpy2.wait());
    removeSpy.clear();
    history2->remove(newData);
    QVERIFY(removeSpy.wait());

    // Sync 2 [out]
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(historyUpdateSpy.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    insertSpy3.clear();
    historyUpdateSpy.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(insertSpy3.isEmpty());
    QVERIFY(historyUpdateSpy.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);
}

void tst_Sync::removeOldInsert_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<TimeLogEntry>("newData");

    TimeLogEntry entry;
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    entry.startTime = QDateTime::fromString("2015-11-01T15:00:00+0200", Qt::ISODate);
    QTest::newRow("empty db") << 0 << entry;

    entry.startTime = QDateTime::fromString("2015-11-01T10:30:00+0200", Qt::ISODate);
    QTest::newRow("1 entry, begin") << 1 << entry;

    entry.startTime = QDateTime::fromString("2015-11-01T11:30:00+0200", Qt::ISODate);
    QTest::newRow("1 entry, end") << 1 << entry;

    entry.startTime = QDateTime::fromString("2015-11-01T11:30:00+0200", Qt::ISODate);
    QTest::newRow("2 entries") << 2 << entry;

    entry.startTime = QDateTime::fromString("2015-11-01T23:30:00+0200", Qt::ISODate);
    QTest::newRow("6 entries") << 6 << entry;
}

void tst_Sync::removeOldRemove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData().mid(0, initialEntries));

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

    QSignalSpy removeSpy1(history1, SIGNAL(dataRemoved(TimeLogEntry)));
    QSignalSpy removeSpy2(history2, SIGNAL(dataRemoved(TimeLogEntry)));
    QSignalSpy removeSpy3(history3, SIGNAL(dataRemoved(TimeLogEntry)));

    QSignalSpy historyUpdateSpy(history3, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

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

    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3. wait());

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
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(!removeSpy3.isEmpty() || removeSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkRemove, removeSpy3, historyUpdateSpy, origData, index);

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    removeSpy3.clear();
    historyUpdateSpy.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(removeSpy3.isEmpty());
    QVERIFY(historyUpdateSpy.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);
}

void tst_Sync::removeOldRemove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries, first") << 2 << 0;
    QTest::newRow("2 entries, last") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 3;
}

void tst_Sync::editOldEdit()
{
    QVector<TimeLogEntry> origData(defaultData());

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

    QSignalSpy historyUpdateSpy1(history1, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy historyUpdateSpy2(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy historyUpdateSpy3(history3, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

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

    // Sync 3 [in]
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3. wait());

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
    syncer2->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy3.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(!historyUpdateSpy3.isEmpty() || historyUpdateSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkEdit, historyUpdateSpy3, origData, fields, index);

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy3.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(historyUpdateSpy3.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);
}

void tst_Sync::editOldEdit_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData1");
    QTest::addColumn<TimeLogEntry>("newData2");

    int index;
    TimeLogEntry entry1;
    TimeLogEntry entry2;

    index = 4;
    entry1 = defaultData().at(index);
    entry1.category = "CategoryOld";
    entry2 = defaultData().at(index);
    entry2.category = "CategoryNew";
    QTest::newRow("category") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultData().at(index);
    entry1.comment = "Test comment old";
    entry2 = defaultData().at(index);
    entry2.comment = "Test comment new";
    QTest::newRow("comment") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultData().at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry2 = defaultData().at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    QTest::newRow("start") << index << entry1 << entry2;

    index = 3;
    entry1 = defaultData().at(index);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultData().at(index);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("category & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultData().at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry2 = defaultData().at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultData().at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry1.comment = "Test comment iold";
    entry2 = defaultData().at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    entry2.comment = "Test comment new";
    QTest::newRow("start & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultData().at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultData().at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("all") << index << entry1 << entry2;
}

void tst_Sync::editOldRemove()
{
    QVector<TimeLogEntry> origData(defaultData());

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

    QSignalSpy removeSpy1(history1, SIGNAL(dataRemoved(TimeLogEntry)));
    QSignalSpy removeSpy3(history3, SIGNAL(dataRemoved(TimeLogEntry)));

    QSignalSpy historyUpdateSpy2(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy historyUpdateSpy3(history3, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

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

    // Sync 3 [in]
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3. wait());

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

    removeSpy1.clear();
    history1->remove(historyData.at(index));
    QVERIFY(removeSpy1.wait());

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

    // Sync 3 [in]
    historyUpdateSpy3.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(!historyUpdateSpy3.isEmpty() || historyUpdateSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkEdit, historyUpdateSpy3, origData, fields, index);

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    removeSpy3.clear();
    historyUpdateSpy3.clear();
    syncer3->sync(QUrl::fromLocalFile(syncDir->path()));
    QVERIFY(syncSpy3.wait());
    QVERIFY(removeSpy3.isEmpty());
    QVERIFY(historyUpdateSpy3.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);
}

void tst_Sync::editOldRemove_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultData().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultData().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultData().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;
}

QTEST_MAIN(tst_Sync)
#include "tst_sync.moc"
