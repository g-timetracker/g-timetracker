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

#include <QtTest/QtTest>

#include <QTemporaryDir>

#include "tst_common.h"
#include "DataSyncer.h"
#include "TimeLogCategoryTreeNode.h"
#include "TimeLogDefaultCategories.h"

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
    void entryInsert();
    void entryInsert_data();
    void entryRemove();
    void entryRemove_data();
    void entryEdit();
    void entryEdit_data();

    void categoryAdd();
    void categoryAdd_data();
    void categoryRemove();
    void categoryRemove_data();
    void categoryEdit();
    void categoryEdit_data();
    void populateCategories();
    void populateCategories_data();

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
    syncDir = new QTemporaryDir();
    Q_CHECK_PTR(syncDir);
    QVERIFY(syncDir->isValid());

    dataDir1 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir1);
    QVERIFY(dataDir1->isValid());
    history1 = new TimeLogHistory;
    Q_CHECK_PTR(history1);
    QVERIFY(history1->init(dataDir1->path()));
    syncer1 = new DataSyncer(history1);
    Q_CHECK_PTR(syncer1);
    syncer1->init(dataDir1->path());
    syncer1->setNoPack(true);
    syncer1->setAutoSync(false);
    syncer1->setSyncPath(QUrl::fromLocalFile(syncDir->path()));

    dataDir2 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir2);
    QVERIFY(dataDir2->isValid());
    history2 = new TimeLogHistory;
    Q_CHECK_PTR(history2);
    QVERIFY(history2->init(dataDir2->path()));
    syncer2 = new DataSyncer(history2);
    Q_CHECK_PTR(syncer2);
    syncer2->init(dataDir2->path());
    syncer2->setNoPack(true);
    syncer2->setAutoSync(false);
    syncer2->setSyncPath(QUrl::fromLocalFile(syncDir->path()));

    dataDir3 = new QTemporaryDir();
    Q_CHECK_PTR(dataDir3);
    QVERIFY(dataDir3->isValid());
    history3 = new TimeLogHistory;
    Q_CHECK_PTR(history3);
    QVERIFY(history3->init(dataDir3->path()));
    syncer3 = new DataSyncer(history3);
    Q_CHECK_PTR(syncer3);
    syncer3->init(dataDir3->path());
    syncer3->setNoPack(true);
    syncer3->setAutoSync(false);
    syncer3->setSyncPath(QUrl::fromLocalFile(syncDir->path()));
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
    history1->deinit();
    delete history1;
    history1 = Q_NULLPTR;
    delete dataDir1;
    dataDir1 = Q_NULLPTR;

    delete syncer2;
    syncer2 = Q_NULLPTR;
    history2->deinit();
    delete history2;
    history2 = Q_NULLPTR;
    delete dataDir2;
    dataDir2 = Q_NULLPTR;

    delete syncer3;
    syncer3 = Q_NULLPTR;
    history3->deinit();
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
    qRegisterMetaType<QVector<TimeLogSyncDataEntry> >();
    qRegisterMetaType<QVector<TimeLogSyncDataCategory> >();
    qRegisterMetaType<QSharedPointer<TimeLogCategoryTreeNode> >();
    qRegisterMetaType<QMap<QDateTime,QByteArray> >();
    qRegisterMetaType<TimeLogCategory>();

    oldCategoryFilter = QLoggingCategory::installFilter(Q_NULLPTR);
    QLoggingCategory::installFilter(syncerCategoryFilter);
    qSetMessagePattern("[%{time}] <%{category}> %{type} (%{file}:%{line}, %{function}) %{message}");
}

void tst_Sync::import()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, entriesCount));

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
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origData);

    // Sync 2 [in]
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, origData);

    // Sync 1 [in]
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_Sync::import_data()
{
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("empty db") << 0;
    QTest::newRow("1 entry") << 1;
    QTest::newRow("2 entries") << 2;
    QTest::newRow("6 entries") << 6;
}

void tst_Sync::entryInsert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

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

        syncer1->sync();
        QVERIFY(syncSpy1.wait());

        syncer2->sync();
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
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    insertSpy2.clear();
    historyUpdateSpy.clear();
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkInsert, insertSpy2, historyUpdateSpy, origData, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_Sync::entryInsert_data()
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

void tst_Sync::entryRemove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

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

        syncer1->sync();
        QVERIFY(syncSpy1.wait());

        syncer2->sync();
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
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    removeSpy2.clear();
    historyUpdateSpy.clear();
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkRemove, removeSpy2, historyUpdateSpy, origData, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_Sync::entryRemove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries, first") << 2 << 0;
    QTest::newRow("2 entries, last") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 3;
}

void tst_Sync::entryEdit()
{
    QVector<TimeLogEntry> origData(defaultEntries());

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

    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    syncer2->sync();
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
    if (fields == TimeLogHistory::NoFields) {
        entry.startTime.addSecs(1);
        historyUpdateSpy1.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy1.wait());

        entry.startTime.addSecs(-1);
        historyUpdateSpy1.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy1.wait());
        origData[index] = entry;
    } else {
        historyUpdateSpy1.clear();
        history1->edit(entry, fields);
        QVERIFY(historyUpdateSpy1.wait());
        origData[index] = entry;
    }

    // Sync 1 [out]
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    historyUpdateSpy2.clear();
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkEdit, historyUpdateSpy2, origData, fields, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_Sync::entryEdit_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    QTest::newRow("nothing (mtime only)") << index << entry;
}

void tst_Sync::categoryAdd()
{
    QFETCH(int, initialEntries);
    QFETCH(int, initialCategories);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, initialCategories));

    QFETCH(QString, categoryName);
    QFETCH(QVariantMap, categoryData);
    TimeLogCategory category(QUuid::createUuid(), TimeLogCategoryData(categoryName, categoryData));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy categoriesSpy(history2, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));

    checkFunction(importSyncData, history1, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    syncer2->sync();
    QVERIFY(syncSpy2. wait());

    if (categoryName.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    } else if (categoryName != "CategoryNew" && initialEntries <= initialCategories) {
        QTest::ignoreMessage(QtCriticalMsg,
                             QRegularExpression(QString("Category '%1' already exists").arg(categoryName)));
    }
    history1->addCategory(category);
    if (categoryName.isEmpty()
        || (categoryName != "CategoryNew" && initialEntries <= initialCategories)) {
        QVERIFY(historyErrorSpy1.wait());
        QVERIFY(historyOutdateSpy1.isEmpty());
    } else {
        QVERIFY(historyErrorSpy1.isEmpty());
        QVERIFY(historyOutdateSpy1.isEmpty());

        origCategories.append(category);
    }

    // Sync 1 [out]
    historyOutdateSpy1.clear();
    historyErrorSpy1.clear();
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    categoriesSpy.clear();
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(historyOutdateSpy2.isEmpty());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());

    if (categoryName.isEmpty()
        || (categoryName != "CategoryNew"
            && (categoryData.isEmpty()
                || initialEntries <= initialCategories))) {
        // Adding category with data, that is already available as entry-only should emit a signal
        QVERIFY(categoriesSpy.isEmpty());
    } else {
        QVERIFY(!categoriesSpy.isEmpty() || categoriesSpy.wait());
    }

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history1, origCategories);

    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history2, origCategories);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_Sync::categoryAdd_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("initialCategories");
    QTest::addColumn<QString>("categoryName");
    QTest::addColumn<QVariantMap>("categoryData");

    QVariantMap data;

    auto addTest = [](const QString &info, int initialEntries, int initialCategories,
            const QString &name, const QVariantMap data)
    {
        QTest::newRow(info.toLocal8Bit()) << initialEntries << initialCategories << name << data;
    };

    auto addTestSet = [&addTest, &data](const QString &prefix)
    {
        addTest(QString("%1, empty category").arg(prefix), 6, 6, "", data);
        addTest(QString("%1, empty category, entry-only").arg(prefix), 6, 0, "", data);
        addTest(QString("%1, different category").arg(prefix), 6, 6, "CategoryNew", data);
        addTest(QString("%1, different category, entry-only").arg(prefix), 6, 0, "CategoryNew", data);
        addTest(QString("%1, different category, no entries").arg(prefix), 0, 6, "CategoryNew", data);
        addTest(QString("%1, same category").arg(prefix), 6, 6, "Category4", data);
        addTest(QString("%1, same category, entry-only").arg(prefix), 6, 0, "Category4", data);
        addTest(QString("%1, same category, no entries").arg(prefix), 0, 6, "Category4", data);
        addTest(QString("%1, 1 entry").arg(prefix), 1, 1, "CategoryNew", data);
        addTest(QString("%1, 1 entry, entry-only").arg(prefix), 1, 0, "CategoryNew", data);
        addTest(QString("%1, 2 entries, first").arg(prefix), 2, 2, "CategoryNew", data);
        addTest(QString("%1, 2 entries, first, entry-only").arg(prefix), 2, 0, "CategoryNew", data);
        addTest(QString("%1, 2 entries, last").arg(prefix), 2, 2, "CategoryNew", data);
        addTest(QString("%1, 2 entries, last, entry-only").arg(prefix), 2, 0, "CategoryNew", data);
        addTest(QString("%1, all entries").arg(prefix), 6, 6, "CategoryNew", data);
        addTest(QString("%1, all entries, entry-only").arg(prefix), 6, 0, "CategoryNew", data);
    };

    data.clear();
    addTestSet("empty data");

    data.clear();
    data.insert("comment", QString("Test comment"));
    addTestSet("comment");
}

void tst_Sync::categoryRemove()
{
    QFETCH(int, initialEntries);
    QFETCH(int, initialCategories);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, initialCategories));

    QFETCH(int, index);
    QString categoryName(index == -1 ? "" : defaultCategories().at(index).name);

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy categoriesSpy(history2, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));

    checkFunction(importSyncData, history1, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    syncer2->sync();
    QVERIFY(syncSpy2. wait());

    if (categoryName.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    } else if (qMax(initialCategories, initialEntries) <= index) {
        QTest::ignoreMessage(QtCriticalMsg,
                             QRegularExpression(QString("No such category: %1").arg(categoryName)));
    }
    history1->removeCategory(categoryName);
    if (categoryName.isEmpty() || qMax(initialCategories, initialEntries) <= index) {
        QVERIFY(historyErrorSpy1.wait());
        QVERIFY(historyOutdateSpy1.isEmpty());
    } else {
        QVERIFY(historyErrorSpy1.isEmpty());
        QVERIFY(historyOutdateSpy1.isEmpty());

        if (index < initialCategories && index >= 0) {
            origCategories.remove(index);
        }
    }

    // Sync 1 [out]
    historyOutdateSpy1.clear();
    historyErrorSpy1.clear();
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    categoriesSpy.clear();
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(historyOutdateSpy2.isEmpty());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());

    if (categoryName.isEmpty() || initialEntries >= index || initialCategories <= index) {
        QVERIFY(categoriesSpy.isEmpty());
    } else {    // Only deleting category without entries should emit signal
        QVERIFY(!categoriesSpy.isEmpty() || categoriesSpy.wait());
    }

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history1, origCategories);

    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history2, origCategories);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_Sync::categoryRemove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("initialCategories");
    QTest::addColumn<int>("index");

    auto addTest = [](const QString &info, int initialEntries, int initialCategories, int index)
    {
        QTest::newRow(info.toLocal8Bit()) << initialEntries << initialCategories << index;
    };

    auto addTestSet = [&addTest]()
    {
        addTest(QString("empty category"), 6, 6, -1);
        addTest(QString("empty category, entry-only"), 6, 0, -1);
        addTest(QString("category"), 6, 6, 3);
        addTest(QString("category, entry-only"), 6, 0, 3);
        addTest(QString("category, no entries"), 0, 6, 3);
        addTest(QString("non-existing category"), 3, 3, 3);
        addTest(QString("non-existing, entry-only"), 3, 0, 3);
        addTest(QString("non-existing, no entries"), 0, 3, 3);
        addTest(QString("1 entry"), 1, 1, 0);
        addTest(QString("1 entry, entry-only"), 1, 0, 0);
        addTest(QString("2 entries, first"), 2, 2, 0);
        addTest(QString("2 entries, first, entry-only"), 2, 0, 0);
        addTest(QString("2 entries, last"), 2, 2, 1);
        addTest(QString("2 entries, last, entry-only"), 2, 0, 1);
    };

    addTestSet();
}

void tst_Sync::categoryEdit()
{
    QFETCH(int, initialEntries);
    QFETCH(int, initialCategories);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, initialCategories));

    QFETCH(QString, categoryNameOld);
    QFETCH(QString, categoryNameNew);
    QFETCH(QVariantMap, categoryDataOld);
    QFETCH(QVariantMap, categoryDataNew);
    QFETCH(QVector<int>, indices);
    if (!categoryNameNew.isEmpty()) {
        for (int index: indices) {
            if (index < origData.size()) {
                origData[index].category = categoryNameOld;
            }
        }
        int index = indices.constFirst();
        if (index < origCategories.size()) {
            origCategories[index].name = categoryNameOld;
            origCategories[index].data = categoryDataOld;
        }
    }
    TimeLogCategory category(defaultCategories().at(indices.constFirst()));
    category.name = categoryNameNew;
    category.data = categoryDataNew;

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy historyCategoriesSpy(history2, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));
    QSignalSpy historyUpdateSpy(history2, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    checkFunction(importSyncData, history1, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    syncer2->sync();
    QVERIFY(syncSpy2. wait());

    if (categoryNameNew.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    }
    history1->editCategory(categoryNameOld, category);
    QVector<int> updateIndices;
    if (categoryNameNew.isEmpty()) {
        QVERIFY(historyErrorSpy1.wait());
        QVERIFY(historyOutdateSpy1.isEmpty());
    } else {
        QVERIFY(historyErrorSpy1.isEmpty());
        QVERIFY(historyOutdateSpy1.isEmpty());

        for (int index: indices) {
            if (categoryNameNew != categoryNameOld && index < initialEntries) {
                updateIndices.append(index);
            }
        }

        if (!categoryNameNew.isEmpty()) {
            bool isMerged = false;
            if (categoryNameNew != categoryNameOld
                && std::find_if(origCategories.begin(), origCategories.end(),
                                [categoryNameNew](const TimeLogCategoryData &d) {
                                    return d.name == categoryNameNew;
                                }) != origCategories.end()) {
                origCategories.erase(std::remove_if(origCategories.begin(), origCategories.end(),
                                                    [&categoryNameOld](const TimeLogCategoryData &d) {
                    return d.name == categoryNameOld;
                }), origCategories.end());

                isMerged = true;
            }
            for (int index: indices) {
                if (index < origData.size()) {
                    origData[index].category = categoryNameNew;
                }
            }
            if (!isMerged) {
                int index = indices.constFirst();
                if (index < origCategories.size()) {
                    origCategories[index] = category;
                } else {
                    origCategories.append(category);
                }
            }
        }
    }

    // Sync 1 [out]
    historyOutdateSpy1.clear();
    historyErrorSpy1.clear();
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    historyCategoriesSpy.clear();
    historyUpdateSpy.clear();
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(historyOutdateSpy2.isEmpty());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    if (categoryNameNew.isEmpty()
        || (categoryNameNew == categoryNameOld
            && (categoryDataNew.isEmpty() || categoryDataNew == categoryDataOld)
            && initialEntries > initialCategories)) {
        // Clear data on entry-only category shouldn't emit a signal
        QVERIFY(historyCategoriesSpy.isEmpty());
    } else {
        QVERIFY(!historyCategoriesSpy.isEmpty() || historyCategoriesSpy.wait());
    }
    while (historyUpdateSpy.size() < updateIndices.size()) {
        QVERIFY(historyUpdateSpy.wait());
    }
    QCOMPARE(historyUpdateSpy.size(), updateIndices.size());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history1, origCategories);

    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history2, origCategories);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_Sync::categoryEdit_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("initialCategories");
    QTest::addColumn<QString>("categoryNameOld");
    QTest::addColumn<QString>("categoryNameNew");
    QTest::addColumn<QVariantMap>("categoryDataOld");
    QTest::addColumn<QVariantMap>("categoryDataNew");
    QTest::addColumn<QVector<int> >("indices");

    QVariantMap oldData;
    QVariantMap newData;

    auto addTest = [](const QString &info, int initialEntries, int initialCategories,
            const QString &oldName, const QString &newName, const QVariantMap oldData,
            const QVariantMap newData, const QVector<int> &indices)
    {
        QTest::newRow(info.toLocal8Bit()) << initialEntries << initialCategories << oldName
                                          << newName << oldData << newData << indices;
    };

    auto addTestSet = [&addTest, &oldData, &newData](const QString &prefix)
    {
        addTest(QString("%1, empty category").arg(prefix), 6, 6, "CategoryOld", "", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, empty category, entry-only").arg(prefix), 6, 0, "CategoryOld", "", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, different category").arg(prefix), 6, 6, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, different category, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, different category, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category").arg(prefix), 6, 6, "CategoryOld", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories").arg(prefix), 6, 6, "CategoryOld", "Category4", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, from entry-only").arg(prefix), 6, 1, "CategoryOld", "Category0", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, to entry-only").arg(prefix), 6, 2, "CategoryOld", "Category2", oldData, newData, QVector<int>() << 1 << 2);
        addTest(QString("%1, merge categories, no entries").arg(prefix), 0, 6, "CategoryOld", "Category4", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, 1 entry").arg(prefix), 1, 1, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 0);
        addTest(QString("%1, 1 entry, entry-only").arg(prefix), 1, 0, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 0);
        addTest(QString("%1, 2 entries, first").arg(prefix), 2, 2, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 0);
        addTest(QString("%1, 2 entries, first, entry-only").arg(prefix), 2, 0, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 0);
        addTest(QString("%1, 2 entries, last").arg(prefix), 2, 2, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 1);
        addTest(QString("%1, 2 entries, last, entry-only").arg(prefix), 2, 0, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 1);
        addTest(QString("%1, all entries").arg(prefix), 6, 6, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 0 << 1 << 2 << 3 << 4 << 5);
        addTest(QString("%1, all entries, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryNew", oldData, newData, QVector<int>() << 0 << 1 << 2 << 3 << 4 << 5);
    };

    oldData.clear();
    newData.clear();
    addTestSet("empty data");

    oldData.clear();
    newData.clear();
    oldData.insert("comment", QString("Test comment"));
    addTestSet("clear comment");

    oldData.clear();
    newData.clear();
    newData.insert("comment", QString("Test comment"));
    addTestSet("add comment");

    oldData.clear();
    newData.clear();
    oldData.insert("comment", QString("Test comment"));
    newData.insert("comment", QString("New comment"));
    addTestSet("edit comment");
}

void tst_Sync::populateCategories()
{
    QFETCH(bool, isPopulateCategories1);
    QFETCH(bool, isPopulateCategories2);

    QVector<TimeLogCategory> origCategories(TimeLogDefaultCategories::defaultCategories());
    QVector<QDateTime> mTimes;
    mTimes.insert(0, origCategories.size(), QDateTime::fromMSecsSinceEpoch(0, Qt::UTC));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, mTimes));

    QSignalSpy syncSpy1(syncer1, SIGNAL(synced()));
    QSignalSpy syncSpy2(syncer2, SIGNAL(synced()));

    QSignalSpy syncErrorSpy1(syncer1, SIGNAL(error(QString)));
    QSignalSpy syncErrorSpy2(syncer2, SIGNAL(error(QString)));

    QSignalSpy historyErrorSpy1(history1, SIGNAL(error(QString)));
    QSignalSpy historyErrorSpy2(history2, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy1(history1, SIGNAL(dataOutdated()));
    QSignalSpy historyOutdateSpy2(history2, SIGNAL(dataOutdated()));

    QSignalSpy categoriesSpy(history2, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));

    TimeLogHistory testHistory1;
    QVERIFY(testHistory1.init(dataDir1->path(), QString(), false, isPopulateCategories1));
    TimeLogHistory testHistory2;
    QVERIFY(testHistory2.init(dataDir2->path(), QString(), false, isPopulateCategories2));

    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    syncer2->sync();
    QVERIFY(syncSpy2. wait());

    QFETCH(QVector<TimeLogSyncDataCategory>, newCategories);
    checkFunction(importSyncData, history1, QVector<TimeLogSyncDataEntry>(), newCategories, 1);

    if (!newCategories.isEmpty()) {
        updateDataSet(origCategories, newCategories.constFirst().category);
        updateDataSet(origSyncCategories, newCategories.constFirst());
    }

    // Sync 1 [out]
    historyOutdateSpy1.clear();
    historyErrorSpy1.clear();
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 2 [in]
    categoriesSpy.clear();
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(historyOutdateSpy2.isEmpty());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());

    checkFunction(checkDB, history1, origCategories);
    checkFunction(checkDB, history2, QVector<TimeLogSyncDataEntry>(), origSyncCategories);

    checkFunction(checkDB, history2, origCategories);
    checkFunction(checkDB, history2, QVector<TimeLogSyncDataEntry>(), origSyncCategories);
}

void tst_Sync::populateCategories_data()
{
    QTest::addColumn<bool>("isPopulateCategories1");
    QTest::addColumn<bool>("isPopulateCategories2");
    QTest::addColumn<QVector<TimeLogSyncDataCategory> >("newCategories");

    int index = 5;

    const QVector<TimeLogCategory> defaultCategories(TimeLogDefaultCategories::defaultCategories());

    TimeLogCategory addedCategory;
    addedCategory.name = "CategoryNew";
    addedCategory.uuid = QUuid::createUuid();
    TimeLogSyncDataCategory addedSyncCategory(addedCategory, defaultMTimes().at(index));

    TimeLogCategory editedCategoryName(defaultCategories.at(index));
    addedCategory.name = "CategoryNew";
    addedCategory.uuid = QUuid::createUuid();
    TimeLogSyncDataCategory editedSyncCategoryName(editedCategoryName, defaultMTimes().at(index));

    TimeLogCategory editedCategoryComment(defaultCategories.at(index));
    addedCategory.data.insert("comment", "Test comment");
    addedCategory.uuid = QUuid::createUuid();
    TimeLogSyncDataCategory editedSyncCategoryComment(editedCategoryComment, defaultMTimes().at(index));

    TimeLogCategory removedCategory;
    removedCategory.uuid = defaultCategories.at(index).uuid;
    TimeLogSyncDataCategory removedSyncCategory(removedCategory, defaultMTimes().at(index));

    QTest::newRow("empty db, populate to no populate") << true << false
            << QVector<TimeLogSyncDataCategory>();
    QTest::newRow("empty db, no populate to populate") << false << true
            << QVector<TimeLogSyncDataCategory>();
    QTest::newRow("empty db, populate to populate") << true << true
            << QVector<TimeLogSyncDataCategory>();

    QTest::newRow("add, populate to no populate") << true << false
            << (QVector<TimeLogSyncDataCategory>() << addedSyncCategory);
    QTest::newRow("add, no populate to populate") << false << true
            << (QVector<TimeLogSyncDataCategory>() << addedSyncCategory);
    QTest::newRow("add, populate to populate") << true << true
            << (QVector<TimeLogSyncDataCategory>() << addedSyncCategory);

    QTest::newRow("edit name, populate to no populate") << true << false
            << (QVector<TimeLogSyncDataCategory>() << editedSyncCategoryName);
    QTest::newRow("edit name, no populate to populate") << false << true
            << (QVector<TimeLogSyncDataCategory>() << editedSyncCategoryName);
    QTest::newRow("edit name, populate to populate") << true << true
            << (QVector<TimeLogSyncDataCategory>() << editedSyncCategoryName);

    QTest::newRow("edit comment, populate to no populate") << true << false
            << (QVector<TimeLogSyncDataCategory>() << editedSyncCategoryComment);
    QTest::newRow("edit comment, no populate to populate") << false << true
            << (QVector<TimeLogSyncDataCategory>() << editedSyncCategoryComment);
    QTest::newRow("edit comment, populate to populate") << true << true
            << (QVector<TimeLogSyncDataCategory>() << editedSyncCategoryComment);

    QTest::newRow("remove, populate to no populate") << true << false
            << (QVector<TimeLogSyncDataCategory>() << removedSyncCategory);
    QTest::newRow("remove, no populate to populate") << false << true
            << (QVector<TimeLogSyncDataCategory>() << removedSyncCategory);
    QTest::newRow("remove, populate to populate") << true << true
            << (QVector<TimeLogSyncDataCategory>() << removedSyncCategory);
}

void tst_Sync::bothRemove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

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

        syncer1->sync();
        QVERIFY(syncSpy1.wait());

        syncer2->sync();
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

    QTest::qSleep(1);   // Ensure entries has different mTime
    removeSpy2.clear();
    history2->remove(historyData.at(index));
    QVERIFY(removeSpy2.wait());

    // Sync 2 [out]
    removeSpy2.clear();
    historyUpdateSpy2.clear();
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in]
    removeSpy1.clear();
    historyUpdateSpy1.clear();
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    QVERIFY(removeSpy1.isEmpty());
    QVERIFY(historyUpdateSpy1.isEmpty());

    origData.remove(index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
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
    QVector<TimeLogEntry> origData(defaultEntries());

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
    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    // Sync 2 [in]
    syncer2->sync();
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
    if (fields == TimeLogHistory::NoFields) {
        entry.startTime.addSecs(1);
        historyUpdateSpy1.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy1.wait());

        entry.startTime.addSecs(-1);
        historyUpdateSpy1.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy1.wait());
        origData[index] = entry;
    } else {
        historyUpdateSpy1.clear();
        history1->edit(entry, fields);
        QVERIFY(historyUpdateSpy1.wait());
    }

    QTest::qSleep(1);   // Ensure entries has different mTime
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
    if (fields == TimeLogHistory::NoFields) {
        entry.startTime.addSecs(1);
        historyUpdateSpy2.clear();
        history2->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy2.wait());

        entry.startTime.addSecs(-1);
        historyUpdateSpy2.clear();
        history2->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy2.wait());
        origData[index] = entry;
    } else {
        historyUpdateSpy2.clear();
        history2->edit(entry, fields);
        QVERIFY(historyUpdateSpy2.wait());
        origData[index] = entry;
    }

    // Sync 2 [out]
    historyUpdateSpy2.clear();
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in]
    historyUpdateSpy1.clear();
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkEdit, historyUpdateSpy1, origData, fields, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
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
    entry1 = defaultEntries().at(index);
    entry1.category = "CategoryOld";
    entry2 = defaultEntries().at(index);
    entry2.category = "CategoryNew";
    QTest::newRow("category") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultEntries().at(index);
    entry1.comment = "Test comment old";
    entry2 = defaultEntries().at(index);
    entry2.comment = "Test comment new";
    QTest::newRow("comment") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultEntries().at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry2 = defaultEntries().at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    QTest::newRow("start") << index << entry1 << entry2;

    index = 3;
    entry1 = defaultEntries().at(index);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultEntries().at(index);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("category & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultEntries().at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry2 = defaultEntries().at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultEntries().at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry1.comment = "Test comment old";
    entry2 = defaultEntries().at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    entry2.comment = "Test comment new";
    QTest::newRow("start & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultEntries().at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultEntries().at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("all") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultEntries().at(index);
    entry2 = defaultEntries().at(index);
    QTest::newRow("nothing (mtime only)") << index << entry1 << entry2;
}

void tst_Sync::editRemove()
{
    QVector<TimeLogEntry> origData(defaultEntries());

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

    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    syncer2->sync();
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
    if (fields == TimeLogHistory::NoFields) {
        entry.startTime.addSecs(1);
        historyUpdateSpy.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy.wait());

        entry.startTime.addSecs(-1);
        historyUpdateSpy.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy.wait());
        origData[index] = entry;
    } else {
        historyUpdateSpy.clear();
        history1->edit(entry, fields);
        QVERIFY(historyUpdateSpy.wait());
    }

    QTest::qSleep(1);   // Ensure entries has different mTime
    removeSpy2.clear();
    history2->remove(historyData.at(index));
    QVERIFY(removeSpy2.wait());

    // Sync 2 [out]
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in]
    removeSpy1.clear();
    historyUpdateSpy.clear();
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkRemove, removeSpy1, historyUpdateSpy, origData, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_Sync::editRemove_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    QTest::newRow("nothing (mtime only)") << index << entry;
}

void tst_Sync::removeEdit()
{
    QVector<TimeLogEntry> origData(defaultEntries());

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

    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    syncer2->sync();
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

    QTest::qSleep(1);   // Ensure entries has different mTime
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
    if (fields == TimeLogHistory::NoFields) {
        entry.startTime.addSecs(1);
        historyUpdateSpy2.clear();
        history2->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy2.wait());

        entry.startTime.addSecs(-1);
        historyUpdateSpy2.clear();
        history2->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy2.wait());
        origData[index] = entry;
    } else {
        historyUpdateSpy2.clear();
        history2->edit(entry, fields);
        QVERIFY(historyUpdateSpy2.wait());
        origData[index] = entry;
    }

    // Sync 2 [out]
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in]
    insertSpy.clear();
    removeSpy.clear();
    historyUpdateSpy1.clear();
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    QVERIFY(removeSpy.isEmpty());
    checkFunction(checkInsert, insertSpy, historyUpdateSpy1, origData, index);

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
}

void tst_Sync::removeEdit_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    QTest::newRow("nothing (mtime only)") << index << entry;
}

void tst_Sync::removeOldEdit()
{
    QVector<TimeLogEntry> origData(defaultEntries());

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
    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    // Sync 2 [in]
    syncer2->sync();
    QVERIFY(syncSpy2. wait());

    // Sync 3 [in]
    syncer3->sync();
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
    if (fields == TimeLogHistory::NoFields) {
        entry.startTime.addSecs(1);
        historyUpdateSpy1.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy1.wait());

        entry.startTime.addSecs(-1);
        historyUpdateSpy1.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy1.wait());
        origData[index] = entry;
    } else {
        historyUpdateSpy1.clear();
        history1->edit(entry, fields);
        QVERIFY(historyUpdateSpy1.wait());
    }

    QTest::qSleep(1);   // Ensure entries has different mTime
    removeSpy2.clear();
    history2->remove(historyData.at(index));
    QVERIFY(removeSpy2.wait());

    // Sync 2 [out]
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 3 [in]
    removeSpy3.clear();
    historyUpdateSpy3.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(!removeSpy3.isEmpty() || removeSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkRemove, removeSpy3, historyUpdateSpy3, origData, index);

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy3.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(historyUpdateSpy3.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history3, origSyncEntries, origSyncCategories);
}

void tst_Sync::removeOldEdit_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    QTest::newRow("nothing (mtime only)") << index << entry;
}

void tst_Sync::removeOldInsert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

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

        syncer1->sync();
        QVERIFY(syncSpy1.wait());

        syncer2->sync();
        QVERIFY(syncSpy2. wait());

        syncer3->sync();
        QVERIFY(syncSpy3. wait());
    }

    QFETCH(TimeLogEntry, newData);

    newData.uuid = QUuid::createUuid();
    insertSpy1.clear();
    history1->insert(newData);
    QVERIFY(insertSpy1.wait());

    QTest::qSleep(1);   // Ensure entries has different mTime
    insertSpy2.clear();
    history2->insert(newData);
    QVERIFY(insertSpy2.wait());
    removeSpy.clear();
    history2->remove(newData);
    QVERIFY(removeSpy.wait());

    // Sync 2 [out]
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(historyUpdateSpy.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    insertSpy3.clear();
    historyUpdateSpy.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(insertSpy3.isEmpty());
    QVERIFY(historyUpdateSpy.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history3, origSyncEntries, origSyncCategories);
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

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

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

    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    syncer2->sync();
    QVERIFY(syncSpy2. wait());

    syncer3->sync();
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

    QTest::qSleep(1);   // Ensure entries has different mTime
    removeSpy2.clear();
    history2->remove(historyData.at(index));
    QVERIFY(removeSpy2.wait());

    // Sync 2 [out]
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(!removeSpy3.isEmpty() || removeSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkRemove, removeSpy3, historyUpdateSpy, origData, index);

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    removeSpy3.clear();
    historyUpdateSpy.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(removeSpy3.isEmpty());
    QVERIFY(historyUpdateSpy.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history3, origSyncEntries, origSyncCategories);
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
    QVector<TimeLogEntry> origData(defaultEntries());

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
    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    // Sync 2 [in]
    syncer2->sync();
    QVERIFY(syncSpy2. wait());

    // Sync 3 [in]
    syncer3->sync();
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
    if (fields == TimeLogHistory::NoFields) {
        entry.startTime.addSecs(1);
        historyUpdateSpy1.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy1.wait());

        entry.startTime.addSecs(-1);
        historyUpdateSpy1.clear();
        history1->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy1.wait());
        origData[index] = entry;
    } else {
        historyUpdateSpy1.clear();
        history1->edit(entry, fields);
        QVERIFY(historyUpdateSpy1.wait());
    }

    QTest::qSleep(1);   // Ensure entries has different mTime
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
    if (fields == TimeLogHistory::NoFields) {
        entry.startTime.addSecs(1);
        historyUpdateSpy2.clear();
        history2->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy2.wait());

        entry.startTime.addSecs(-1);
        historyUpdateSpy2.clear();
        history2->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy2.wait());
        origData[index] = entry;
    } else {
        historyUpdateSpy2.clear();
        history2->edit(entry, fields);
        QVERIFY(historyUpdateSpy2.wait());
        origData[index] = entry;
    }

    // Sync 2 [out]
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy3.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(!historyUpdateSpy3.isEmpty() || historyUpdateSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkEdit, historyUpdateSpy3, origData, fields, index);

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy3.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(historyUpdateSpy3.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history3, origSyncEntries, origSyncCategories);
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
    entry1 = defaultEntries().at(index);
    entry1.category = "CategoryOld";
    entry2 = defaultEntries().at(index);
    entry2.category = "CategoryNew";
    QTest::newRow("category") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultEntries().at(index);
    entry1.comment = "Test comment old";
    entry2 = defaultEntries().at(index);
    entry2.comment = "Test comment new";
    QTest::newRow("comment") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultEntries().at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry2 = defaultEntries().at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    QTest::newRow("start") << index << entry1 << entry2;

    index = 3;
    entry1 = defaultEntries().at(index);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultEntries().at(index);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("category & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultEntries().at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry2 = defaultEntries().at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry1 << entry2;

    index = 2;
    entry1 = defaultEntries().at(index);
    entry1.startTime = entry1.startTime.addSecs(-100);
    entry1.comment = "Test comment old";
    entry2 = defaultEntries().at(index);
    entry2.startTime = entry2.startTime.addSecs(-50);
    entry2.comment = "Test comment new";
    QTest::newRow("start & comment") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultEntries().at(index);
    entry1.startTime = entry1.startTime.addSecs(1000);
    entry1.category = "CategoryOld";
    entry1.comment = "Test comment old";
    entry2 = defaultEntries().at(index);
    entry2.startTime = entry2.startTime.addSecs(500);
    entry2.category = "CategoryNew";
    entry2.comment = "Test comment new";
    QTest::newRow("all") << index << entry1 << entry2;

    index = 1;
    entry1 = defaultEntries().at(index);
    entry2 = defaultEntries().at(index);
    QTest::newRow("nothing (mtime only)") << index << entry1 << entry2;
}

void tst_Sync::editOldRemove()
{
    QVector<TimeLogEntry> origData(defaultEntries());

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
    syncer1->sync();
    QVERIFY(syncSpy1.wait());

    // Sync 2 [in]
    syncer2->sync();
    QVERIFY(syncSpy2. wait());

    // Sync 3 [in]
    syncer3->sync();
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

    QTest::qSleep(1);   // Ensure entries has different mTime
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
    if (fields == TimeLogHistory::NoFields) {
        entry.startTime.addSecs(1);
        historyUpdateSpy2.clear();
        history2->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy2.wait());

        entry.startTime.addSecs(-1);
        historyUpdateSpy2.clear();
        history2->edit(entry, TimeLogHistory::StartTime);
        QVERIFY(historyUpdateSpy2.wait());
        origData[index] = entry;
    } else {
        historyUpdateSpy2.clear();
        history2->edit(entry, fields);
        QVERIFY(historyUpdateSpy2.wait());
        origData[index] = entry;
    }

    // Sync 2 [out]
    syncer2->sync();
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 3 [in]
    historyUpdateSpy3.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(!historyUpdateSpy3.isEmpty() || historyUpdateSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkEdit, historyUpdateSpy3, origData, fields, index);

    checkFunction(checkDB, history3, origData);

    // Sync 1 [in-out]
    syncer1->sync();
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    // Sync 3 [in]
    removeSpy3.clear();
    historyUpdateSpy3.clear();
    syncer3->sync();
    QVERIFY(syncSpy3.wait());
    QVERIFY(removeSpy3.isEmpty());
    QVERIFY(historyUpdateSpy3.isEmpty());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history3, origData);

    QVector<TimeLogSyncDataEntry> origSyncEntries;
    QVector<TimeLogSyncDataCategory> origSyncCategories;
    checkFunction(extractSyncData, history1, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history2, origSyncEntries, origSyncCategories);
    checkFunction(checkDB, history3, origSyncEntries, origSyncCategories);
}

void tst_Sync::editOldRemove_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 4;
    TimeLogEntry entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    QTest::newRow("category") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.comment = "Test comment";
    QTest::newRow("comment") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    QTest::newRow("start") << index << entry;

    index = 3;
    entry = defaultEntries().at(index);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("category & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    QTest::newRow("start & category") << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(-100);
    entry.comment = "Test comment";
    QTest::newRow("start & comment") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = entry.startTime.addSecs(1000);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("all") << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    QTest::newRow("nothing (mtime only)") << index << entry;
}

QTEST_MAIN(tst_Sync)
#include "tst_sync.moc"
