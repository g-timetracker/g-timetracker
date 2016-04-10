#include <QtTest/QtTest>

#include <QTemporaryDir>

#include "tst_common.h"
#include "TimeLogCategoryTreeNode.h"

QTemporaryDir *dataDir = Q_NULLPTR;
TimeLogHistory *history = Q_NULLPTR;

class tst_DB : public QObject
{
    Q_OBJECT

public:
    tst_DB();
    virtual ~tst_DB();

private slots:
    void init();
    void cleanup();
    void initTestCase();

    void import();
    void import_data();
    void entryInsert();
    void entryInsert_data();
    void entryInsertConflict();
    void entryInsertConflict_data();
    void entryRemove();
    void entryRemove_data();
    void entryEdit();
    void entryEdit_data();
    void entryEditConflict();
    void entryEditConflict_data();

    void categoryAdd();
    void categoryAdd_data();
    void categoryRemove();
    void categoryRemove_data();
    void categoryEdit();
    void categoryEdit_data();
    void categoryAddName();
    void categoryAddName_data();
    void categoryEditName();
    void categoryEditName_data();

    void undoEntryInsert();
    void undoEntryInsert_data();
    void undoEntryRemove();
    void undoEntryRemove_data();
    void undoEntryEdit();
    void undoEntryEdit_data();
    void undoEntryMultiple();

    void undoCategoryAdd();
    void undoCategoryAdd_data();
    void undoCategoryRemove();
    void undoCategoryRemove_data();
    void undoCategoryEdit();
    void undoCategoryEdit_data();

    void hashes();
    void hashes_data();
    void hashesUpdate();
    void hashesUpdate_data();
    void hashesOld();
    void hashesOld_data();
};

tst_DB::tst_DB()
{
}

tst_DB::~tst_DB()
{
}

void tst_DB::init()
{
    dataDir = new QTemporaryDir();
    Q_CHECK_PTR(dataDir);
    QVERIFY(dataDir->isValid());
    history = new TimeLogHistory;
    Q_CHECK_PTR(history);
    QVERIFY(history->init(dataDir->path()));
}

void tst_DB::cleanup()
{
    if (QTest::currentTestFailed()) {
//        dataDir->setAutoRemove(false);
    }
    delete history;
    history = Q_NULLPTR;
    delete dataDir;
    dataDir = Q_NULLPTR;
}

void tst_DB::initTestCase()
{
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();
    qRegisterMetaType<QSharedPointer<TimeLogCategoryTreeNode> >();
    qRegisterMetaType<QMap<QDateTime,QByteArray> >();
    qRegisterMetaType<TimeLogCategory>();
    qRegisterMetaType<QVector<TimeLogSyncDataCategory> >();

    qSetMessagePattern("[%{time}] <%{category}> %{type} (%{file}:%{line}, %{function}) %{message}");
}

void tst_DB::import()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, entriesCount));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(history->size(), origData.size());
    QVERIFY(compareData(importSpy.constFirst().at(0).value<QVector<TimeLogEntry> >(), origData));

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::import_data()
{
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("empty db") << 0;
    QTest::newRow("1 entry") << 1;
    QTest::newRow("2 entries") << 2;
    QTest::newRow("6 entries") << 6;
}

void tst_DB::entryInsert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy insertSpy(history, SIGNAL(dataInserted(TimeLogEntry)));

    if (initialEntries) {
        QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
        history->import(origData);
        QVERIFY(importSpy.wait());
    }

    QFETCH(int, index);
    TimeLogEntry entry;
    QFETCH(QDateTime, insertTime);
    entry.startTime = insertTime;
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    entry.uuid = QUuid::createUuid();
    history->insert(entry);
    QVERIFY(insertSpy.wait());
    QVERIFY(!updateSpy.isEmpty() || updateSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QVERIFY(compareData(insertSpy.constFirst().at(0).value<TimeLogEntry>(), entry));

    origData.insert(index, entry);

    checkFunction(checkInsert, insertSpy, updateSpy, origData, index);

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::entryInsert_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");
    QTest::addColumn<QDateTime>("insertTime");

    QTest::newRow("empty db") << 0 << 0 << QDateTime::fromString("2015-11-01T15:00:00+0200", Qt::ISODate);
    QTest::newRow("1 entry, begin") << 1 << 0 << QDateTime::fromString("2015-11-01T10:30:00+0200", Qt::ISODate);
    QTest::newRow("1 entry, end") << 1 << 1 << QDateTime::fromString("2015-11-01T11:30:00+0200", Qt::ISODate);
    QTest::newRow("2 entries") << 2 << 2 << QDateTime::fromString("2015-11-01T11:30:00+0200", Qt::ISODate);
    QTest::newRow("6 entries") << 6 << 4 << QDateTime::fromString("2015-11-01T23:30:00+0200", Qt::ISODate);
}

void tst_DB::entryInsertConflict()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy dataSpy(history, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy insertSpy(history, SIGNAL(dataInserted(TimeLogEntry)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait());

    id = QDateTime::currentMSecsSinceEpoch();
    history->getHistoryBetween(id);
    QVERIFY(dataSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(dataSpy.constFirst().at(1).toLongLong(), id);
    historyData = dataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

    QFETCH(int, index);
    TimeLogEntry entry;
    entry.startTime = historyData.at(index).startTime;
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    entry.uuid = QUuid::createUuid();
    QTest::ignoreMessage(QtCriticalMsg, QRegularExpression("Fail to execute query: \"UNIQUE constraint failed"));
    history->insert(entry);
    QVERIFY(errorSpy.wait());
    QVERIFY(!outdateSpy.isEmpty() || outdateSpy.wait());
    QVERIFY(insertSpy.isEmpty());
    QVERIFY(updateSpy.isEmpty());

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::entryInsertConflict_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 4;
}

void tst_DB::entryRemove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy dataSpy(history, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy removeSpy(history, SIGNAL(dataRemoved(TimeLogEntry)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    if (initialEntries) {
        QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
        history->import(origData);
        QVERIFY(importSpy.wait());
    }

    id = QDateTime::currentMSecsSinceEpoch();
    history->getHistoryBetween(id);
    QVERIFY(dataSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(dataSpy.constFirst().at(1).toLongLong(), id);
    historyData = dataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

    QFETCH(int, index);
    history->remove(historyData.at(index));
    QVERIFY(removeSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(removeSpy.constFirst().at(0).value<TimeLogEntry>().uuid, origData.at(index).uuid);

    if (initialEntries < 2) {
        QVERIFY(updateSpy.isEmpty());
        return;
    }

    checkFunction(checkRemove, removeSpy, updateSpy, origData, index);

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::entryRemove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries, first") << 2 << 0;
    QTest::newRow("2 entries, last") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 3;
}

void tst_DB::entryEdit()
{
    QVector<TimeLogEntry> origData(defaultEntries());

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy dataSpy(history, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait());

    dataSpy.clear();
    id = QDateTime::currentMSecsSinceEpoch();
    history->getHistoryBetween(id);
    QVERIFY(dataSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(dataSpy.constFirst().at(1).toLongLong(), id);
    historyData = dataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

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
    history->edit(entry, fields);
    QVERIFY(updateSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());

    origData[index] = entry;

    checkFunction(checkEdit, updateSpy, origData, fields, index);

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::entryEdit_data()
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
}

void tst_DB::entryEditConflict()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy dataSpy(history, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));

    qlonglong id;
    QVector<TimeLogEntry> historyData;

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait());

    dataSpy.clear();
    id = QDateTime::currentMSecsSinceEpoch();
    history->getHistoryBetween(id);
    QVERIFY(dataSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(dataSpy.constFirst().at(1).toLongLong(), id);
    historyData = dataSpy.constFirst().at(0).value<QVector<TimeLogEntry> >();

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
    QTest::ignoreMessage(QtCriticalMsg, QRegularExpression("Fail to execute query: \"UNIQUE constraint failed"));
    updateSpy.clear();
    history->edit(entry, fields);
    QVERIFY(errorSpy.wait());
    QVERIFY(!outdateSpy.isEmpty() || outdateSpy.wait());
    QVERIFY(updateSpy.isEmpty());

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::entryEditConflict_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 1;
    TimeLogEntry entry = defaultEntries().at(index);
    entry.startTime = defaultEntries().at(0).startTime;
    QTest::newRow("2 entries, first") << 2 << index << entry;

    index = 0;
    entry = defaultEntries().at(index);
    entry.startTime = defaultEntries().at(1).startTime;
    QTest::newRow("2 entries, last") << 2 << index << entry;

    index = 3;
    entry = defaultEntries().at(index);
    entry.startTime = defaultEntries().at(4).startTime;
    QTest::newRow("6 entries, middle") << 6 << index << entry;

    index = 5;
    entry = defaultEntries().at(index);
    entry.startTime = defaultEntries().at(2).startTime;
    entry.category = "CategoryNew";
    QTest::newRow("6 entries, category") << 6 << index << entry;

    index = 1;
    entry = defaultEntries().at(index);
    entry.startTime = defaultEntries().at(3).startTime;
    entry.comment = "Test comment";
    QTest::newRow("6 entries, comment") << 6 << index << entry;

    index = 2;
    entry = defaultEntries().at(index);
    entry.startTime = defaultEntries().at(0).startTime;
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("6 entries, all") << 6 << index << entry;
}

void tst_DB::categoryAdd()
{
    QFETCH(int, initialEntries);
    QFETCH(int, initialCategories);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, initialCategories));

    QFETCH(QString, categoryName);
    QFETCH(QVariantMap, categoryData);
    TimeLogCategory category(QUuid::createUuid(), TimeLogCategoryData(categoryName, categoryData));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy categoriesSpy(history, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));

    checkFunction(importSyncData, history, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    if (categoryName.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    } else if (categoryName != "CategoryNew" && initialEntries <= initialCategories) {
        QTest::ignoreMessage(QtCriticalMsg,
                             QRegularExpression(QString("Category '%1' already exists").arg(categoryName)));
    }
    categoriesSpy.clear();
    history->addCategory(category);
    if (categoryName.isEmpty()
        || (categoryName != "CategoryNew" && initialEntries <= initialCategories)) {
        QVERIFY(errorSpy.wait());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(categoriesSpy.isEmpty());
    } else {
        if (categoryName != "CategoryNew" && initialEntries > initialCategories) {
            QVERIFY(categoriesSpy.isEmpty());
        } else {
            QVERIFY(categoriesSpy.wait());
        }
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());

        origCategories.append(category);
    }

    checkFunction(checkDB, history, origData);
    checkFunction(checkDB, history, origCategories);

    checkFunction(checkHashes, history, false);
}

void tst_DB::categoryAdd_data()
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

void tst_DB::categoryRemove()
{
    QFETCH(int, initialEntries);
    QFETCH(int, initialCategories);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, initialCategories));

    QFETCH(int, index);
    QString categoryName(index == -1 ? "" : defaultCategories().at(index).name);

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy categoriesSpy(history, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));

    checkFunction(importSyncData, history, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    if (categoryName.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    } else if (qMax(initialCategories, initialEntries) <= index) {
        QTest::ignoreMessage(QtCriticalMsg,
                             QRegularExpression(QString("No such category: %1").arg(categoryName)));
    }
    categoriesSpy.clear();
    history->removeCategory(categoryName);
    if (categoryName.isEmpty() || qMax(initialCategories, initialEntries) <= index) {
        QVERIFY(errorSpy.wait());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(categoriesSpy.isEmpty());
    } else {
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
        if (initialEntries <= index) {  // Only deleting category without entries should emit signal
            QVERIFY(categoriesSpy.wait());
        } else {
            QVERIFY(categoriesSpy.isEmpty());
        }

        if (index < initialCategories && index >= 0) {
            origCategories.remove(index);
        }
    }

    checkFunction(checkDB, history, origData);
    checkFunction(checkDB, history, origCategories);

    checkFunction(checkHashes, history, false);
}

void tst_DB::categoryRemove_data()
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

void tst_DB::categoryEdit()
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
    if (!categoryNameNew.isEmpty() && !indices.isEmpty()) {
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
    TimeLogCategory category(defaultCategories().at(indices.isEmpty() ? 0 : indices.constFirst()));
    category.name = categoryNameNew;
    category.data = categoryDataNew;

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy categoriesSpy(history, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    checkFunction(importSyncData, history, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    if (categoryNameNew.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    } else if (categoryNameOld == "NoCategory") {
        QTest::ignoreMessage(QtCriticalMsg,
                             QRegularExpression(QString("No such category: %1").arg(categoryNameOld)));
    }
    outdateSpy.clear();
    categoriesSpy.clear();
    updateSpy.clear();
    history->editCategory(categoryNameOld, category);
    if (categoryNameNew.isEmpty() || categoryNameOld == "NoCategory") {
        QVERIFY(errorSpy.wait());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(updateSpy.isEmpty());
        QVERIFY(categoriesSpy.isEmpty());
    } else {
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
        if (categoryNameNew == categoryNameOld
            && (categoryDataNew.isEmpty() || categoryDataNew == categoryDataOld)
            && initialEntries > initialCategories) {
            // Clear data on entry-only category shouldn't emit a signal
            QVERIFY(categoriesSpy.isEmpty());
        } else {
            QVERIFY(!categoriesSpy.isEmpty() || categoriesSpy.wait());
        }

        QVector<int> updateIndices;
        for (int index: indices) {
            if (categoryNameNew != categoryNameOld && index < initialEntries) {
                updateIndices.append(index);
            }
        }
        while (updateSpy.size() < updateIndices.size()) {
            QVERIFY(updateSpy.wait());
        }
        QCOMPARE(updateSpy.size(), updateIndices.size());

        bool isMerged = false;
        if (categoryNameNew != categoryNameOld
            && std::find_if(origCategories.begin(), origCategories.end(),
                            [categoryNameNew](const TimeLogCategory &c) {
                                return c.name == categoryNameNew;
                            }) != origCategories.end()) {
            origCategories.erase(std::remove_if(origCategories.begin(), origCategories.end(),
                                                [&categoryNameOld](const TimeLogCategory &c) {
                return c.name == categoryNameOld;
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

        for (int i = 0; i < updateIndices.size(); i++) {
            QVector<TimeLogEntry> updateData = updateSpy.at(i).at(0).value<QVector<TimeLogEntry> >();
            QVector<TimeLogHistory::Fields> updateFields = updateSpy.at(i).at(1).value<QVector<TimeLogHistory::Fields> >();
            QCOMPARE(updateData.size(), 1);
            QCOMPARE(updateData.constFirst().category, origData.at(updateIndices.at(i)).category);
            QCOMPARE(updateFields.constFirst(), TimeLogHistory::Category);
        }

    }

    checkFunction(checkDB, history, origData);
    checkFunction(checkDB, history, origCategories);

    checkFunction(checkHashes, history, false);
}

void tst_DB::categoryEdit_data()
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
        addTest(QString("%1, absent category").arg(prefix), 6, 6, "NoCategory", "CategoryNew", oldData, newData, QVector<int>());
        addTest(QString("%1, absent category, entry-only").arg(prefix), 6, 0, "NoCategory", "CategoryNew", oldData, newData, QVector<int>());
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

void tst_DB::categoryAddName()
{
    QFETCH(int, initialEntries);
    QFETCH(int, initialCategories);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, initialCategories));

    QFETCH(QString, categoryName);
    QFETCH(QString, categoryNameResult);
    QFETCH(QVariantMap, categoryData);
    TimeLogCategory category(QUuid::createUuid(), TimeLogCategoryData(categoryName, categoryData));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy categoriesSpy(history, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));

    checkFunction(importSyncData, history, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    if (categoryNameResult.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    } else if (!categoryName.contains("CategoryNew") && initialEntries <= initialCategories) {
        QTest::ignoreMessage(QtCriticalMsg,
                             QRegularExpression(QString("Category '%1' already exists").arg(categoryNameResult)));
    }
    categoriesSpy.clear();
    history->addCategory(category);
    if (categoryNameResult.isEmpty()
        || (!categoryName.contains("CategoryNew") && initialEntries <= initialCategories)) {
        QVERIFY(errorSpy.wait());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(categoriesSpy.isEmpty());
    } else {
        if (!categoryName.contains("CategoryNew") && initialEntries > initialCategories) {
            QVERIFY(categoriesSpy.isEmpty());
        } else {
            QVERIFY(categoriesSpy.wait());
        }
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());

        TimeLogCategory resultCategory(category);
        resultCategory.name = categoryNameResult;
        origCategories.append(resultCategory);
    }

    checkFunction(checkDB, history, origData);
    checkFunction(checkDB, history, origCategories);

    checkFunction(checkHashes, history, false);
}

void tst_DB::categoryAddName_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("initialCategories");
    QTest::addColumn<QString>("categoryName");
    QTest::addColumn<QString>("categoryNameResult");
    QTest::addColumn<QVariantMap>("categoryData");

    QVariantMap data;

    auto addTest = [](const QString &info, int initialEntries, int initialCategories,
            const QString &name, const QString &resultName, const QVariantMap data)
    {
        QTest::newRow(info.toLocal8Bit()) << initialEntries << initialCategories << name << resultName << data;
    };

    auto addTestSet = [&addTest, &data](const QString &prefix)
    {
        addTest(QString("%1, subcategory, spaces").arg(prefix), 6, 6, "CategoryNew > SubCategory", "CategoryNew > SubCategory", data);
        addTest(QString("%1, subcategory, spaces, entry-only").arg(prefix), 6, 0, "CategoryNew > SubCategory", "CategoryNew > SubCategory", data);
        addTest(QString("%1, subcategory, spaces, no entries").arg(prefix), 0, 6, "CategoryNew > SubCategory", "CategoryNew > SubCategory", data);
        addTest(QString("%1, subcategory, no spaces").arg(prefix), 6, 6, "CategoryNew>SubCategory", "CategoryNew > SubCategory", data);
        addTest(QString("%1, subcategory, no spaces, entry-only").arg(prefix), 6, 0, "CategoryNew>SubCategory", "CategoryNew > SubCategory", data);
        addTest(QString("%1, subcategory, no spaces, no entries").arg(prefix), 0, 6, "CategoryNew>SubCategory", "CategoryNew > SubCategory", data);
        addTest(QString("%1, subsubcategory").arg(prefix), 6, 6, "CategoryNew > SubCategory > SubSubCategory", "CategoryNew > SubCategory > SubSubCategory", data);
        addTest(QString("%1, subsubcategory, entry-only").arg(prefix), 6, 0, "CategoryNew > SubCategory > SubSubCategory", "CategoryNew > SubCategory > SubSubCategory", data);
        addTest(QString("%1, subsubcategory, no entries").arg(prefix), 0, 6, "CategoryNew > SubCategory > SubSubCategory", "CategoryNew > SubCategory > SubSubCategory", data);

        addTest(QString("%1, adjacent separator").arg(prefix), 6, 6, "CategoryNew >", "CategoryNew", data);
        addTest(QString("%1, adjacent separator, entry-only").arg(prefix), 6, 0, "CategoryNew >", "CategoryNew", data);
        addTest(QString("%1, adjacent separator, no entries").arg(prefix), 0, 6, "CategoryNew >", "CategoryNew", data);
        addTest(QString("%1, adjacent separator and space").arg(prefix), 6, 6, "CategoryNew > ", "CategoryNew", data);
        addTest(QString("%1, adjacent separator and space, entry-only").arg(prefix), 6, 0, "CategoryNew > ", "CategoryNew", data);
        addTest(QString("%1, adjacent separator and space, no entries").arg(prefix), 0, 6, "CategoryNew > ", "CategoryNew", data);

        addTest(QString("%1, spaces only").arg(prefix), 6, 6, "   ", "", data);
        addTest(QString("%1, spaces only, entry-only").arg(prefix), 6, 0, "   ", "", data);
        addTest(QString("%1, spaces only, no entries").arg(prefix), 0, 6, "   ", "", data);

        addTest(QString("%1, name with spaces").arg(prefix), 6, 6, "CategoryNew with   spaces", "CategoryNew with   spaces", data);
        addTest(QString("%1, name with spaces, entry-only").arg(prefix), 6, 0, "CategoryNew with   spaces", "CategoryNew with   spaces", data);
        addTest(QString("%1, name with spaces, no entries").arg(prefix), 0, 6, "CategoryNew with   spaces", "CategoryNew with   spaces", data);

        addTest(QString("%1, adjacent spaces, begin").arg(prefix), 6, 6, " CategoryNew", "CategoryNew", data);
        addTest(QString("%1, adjacent spaces, begin, entry-only").arg(prefix), 6, 0, " CategoryNew", "CategoryNew", data);
        addTest(QString("%1, adjacent spaces, begin, no entries").arg(prefix), 0, 6, " CategoryNew", "CategoryNew", data);
        addTest(QString("%1, adjacent spaces, end").arg(prefix), 6, 6, "CategoryNew ", "CategoryNew", data);
        addTest(QString("%1, adjacent spaces, end, entry-only").arg(prefix), 6, 0, "CategoryNew ", "CategoryNew", data);
        addTest(QString("%1, adjacent spaces, end, no entries").arg(prefix), 0, 6, "CategoryNew ", "CategoryNew", data);

        addTest(QString("%1, same category").arg(prefix), 6, 6, "Category4", "Category4", data);
        addTest(QString("%1, same category, entry-only").arg(prefix), 6, 0, "Category4", "Category4", data);
        addTest(QString("%1, same category, no entries").arg(prefix), 0, 6, "Category4", "Category4", data);
        addTest(QString("%1, same category, spaces, begin").arg(prefix), 6, 6, " Category4", "Category4", data);
        addTest(QString("%1, same category, spaces, begin, entry-only").arg(prefix), 6, 0, " Category4", "Category4", data);
        addTest(QString("%1, same category, spaces, begin, no entries").arg(prefix), 0, 6, " Category4", "Category4", data);
        addTest(QString("%1, same category, spaces, end").arg(prefix), 6, 6, "Category4 ", "Category4", data);
        addTest(QString("%1, same category, spaces, end, entry-only").arg(prefix), 6, 0, "Category4 ", "Category4", data);
        addTest(QString("%1, same category, spaces, end, no entries").arg(prefix), 0, 6, "Category4 ", "Category4", data);
    };

    data.clear();
    addTestSet("empty data");

    data.clear();
    data.insert("comment", QString("Test comment"));
    addTestSet("comment");
}

void tst_DB::categoryEditName()
{
    QFETCH(int, initialEntries);
    QFETCH(int, initialCategories);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, initialCategories));

    QFETCH(QString, categoryNameOld);
    QFETCH(QString, categoryNameNew);
    QFETCH(QString, categoryNameResult);
    QFETCH(QVariantMap, categoryDataOld);
    QFETCH(QVariantMap, categoryDataNew);
    QFETCH(QVector<int>, indices);
    if (!categoryNameResult.isEmpty()) {
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

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy categoriesSpy(history, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    checkFunction(importSyncData, history, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    if (categoryNameResult.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    }
    outdateSpy.clear();
    categoriesSpy.clear();
    updateSpy.clear();
    history->editCategory(categoryNameOld, category);
    if (categoryNameResult.isEmpty()) {
        QVERIFY(errorSpy.wait());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(updateSpy.isEmpty());
        QVERIFY(categoriesSpy.isEmpty());
    } else {
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
        if (categoryNameResult == categoryNameOld
            && (categoryDataNew.isEmpty() || categoryDataNew == categoryDataOld)
            && initialEntries > initialCategories) {
            // Clear data on entry-only category shouldn't emit a signal
            QVERIFY(categoriesSpy.isEmpty());
        } else {
            QVERIFY(!categoriesSpy.isEmpty() || categoriesSpy.wait());
        }

        QVector<int> updateIndices;
        for (int index: indices) {
            if (categoryNameResult != categoryNameOld && index < initialEntries) {
                updateIndices.append(index);
            }
        }
        while (updateSpy.size() < updateIndices.size()) {
            QVERIFY(updateSpy.wait());
        }
        QCOMPARE(updateSpy.size(), updateIndices.size());

        bool isMerged = false;
        if (categoryNameResult != categoryNameOld
            && std::find_if(origCategories.begin(), origCategories.end(),
                            [categoryNameResult](const TimeLogCategory &c) {
                                return c.name == categoryNameResult;
                            }) != origCategories.end()) {
            origCategories.erase(std::remove_if(origCategories.begin(), origCategories.end(),
                                                [&categoryNameOld](const TimeLogCategory &c) {
                return c.name == categoryNameOld;
            }), origCategories.end());

            isMerged = true;
        }
        for (int index: indices) {
            if (index < origData.size()) {
                origData[index].category = categoryNameResult;
            }
        }
        if (!isMerged) {
            category.name = categoryNameResult;
            int index = indices.constFirst();
            if (index < origCategories.size()) {
                origCategories[index] = category;
            } else {
                origCategories.append(category);
            }
        }

        for (int i = 0; i < updateIndices.size(); i++) {
            QVector<TimeLogEntry> updateData = updateSpy.at(i).at(0).value<QVector<TimeLogEntry> >();
            QVector<TimeLogHistory::Fields> updateFields = updateSpy.at(i).at(1).value<QVector<TimeLogHistory::Fields> >();
            QCOMPARE(updateData.size(), 1);
            QCOMPARE(updateData.constFirst().category, origData.at(updateIndices.at(i)).category);
            QCOMPARE(updateFields.constFirst(), TimeLogHistory::Category);
        }

    }

    checkFunction(checkDB, history, origData);
    checkFunction(checkDB, history, origCategories);

    checkFunction(checkHashes, history, false);
}

void tst_DB::categoryEditName_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("initialCategories");
    QTest::addColumn<QString>("categoryNameOld");
    QTest::addColumn<QString>("categoryNameNew");
    QTest::addColumn<QString>("categoryNameResult");
    QTest::addColumn<QVariantMap>("categoryDataOld");
    QTest::addColumn<QVariantMap>("categoryDataNew");
    QTest::addColumn<QVector<int> >("indices");

    QVariantMap oldData;
    QVariantMap newData;

    auto addTest = [](const QString &info, int initialEntries, int initialCategories,
            const QString &oldName, const QString &newName, const QString &resultName,
            const QVariantMap oldData, const QVariantMap newData, const QVector<int> &indices)
    {
        QTest::newRow(info.toLocal8Bit()) << initialEntries << initialCategories << oldName
                                          << newName << resultName << oldData << newData << indices;
    };

    auto addTestSet = [&addTest, &oldData, &newData](const QString &prefix)
    {
        addTest(QString("%1, subcategory, spaces").arg(prefix), 6, 6, "CategoryOld", "CategoryNew > SubCategory", "CategoryNew > SubCategory", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, subcategory, spaces, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryNew > SubCategory", "CategoryNew > SubCategory", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, subcategory, spaces, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryNew > SubCategory", "CategoryNew > SubCategory", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, subcategory, no spaces").arg(prefix), 6, 6, "CategoryOld", "CategoryNew>SubCategory", "CategoryNew > SubCategory", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, subcategory, no spaces, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryNew>SubCategory", "CategoryNew > SubCategory", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, subcategory, no spaces, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryNew>SubCategory", "CategoryNew > SubCategory", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, subsubcategory").arg(prefix), 6, 6, "CategoryOld", "CategoryNew > SubCategory > SubSubCategory", "CategoryNew > SubCategory > SubSubCategory", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, subsubcategory, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryNew > SubCategory > SubSubCategory", "CategoryNew > SubCategory > SubSubCategory", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, subsubcategory, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryNew > SubCategory > SubSubCategory", "CategoryNew > SubCategory > SubSubCategory", oldData, newData, QVector<int>() << 1 << 3);

        addTest(QString("%1, adjacent separator").arg(prefix), 6, 6, "CategoryOld", "CategoryNew >", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent separator, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryNew >", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent separator, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryNew >", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent separator and space").arg(prefix), 6, 6, "CategoryOld", "CategoryNew > ", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent separator and space, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryNew > ", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent separator and space, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryNew > ", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);

        addTest(QString("%1, spaces only").arg(prefix), 6, 6, "CategoryOld", "   ", "", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, spaces only, entry-only").arg(prefix), 6, 0, "CategoryOld", "   ", "", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, spaces only, no entries").arg(prefix), 0, 6, "CategoryOld", "   ", "", oldData, newData, QVector<int>() << 1 << 3);

        addTest(QString("%1, name with spaces").arg(prefix), 6, 6, "CategoryOld", "CategoryNew with   spaces", "CategoryNew with   spaces", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, name with spaces, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryNew with   spaces", "CategoryNew with   spaces", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, name with spaces, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryNew with   spaces", "CategoryNew with   spaces", oldData, newData, QVector<int>() << 1 << 3);

        addTest(QString("%1, adjacent spaces, begin").arg(prefix), 6, 6, "CategoryOld", " CategoryNew", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent spaces, begin, entry-only").arg(prefix), 6, 0, "CategoryOld", " CategoryNew", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent spaces, begin, no entries").arg(prefix), 0, 6, "CategoryOld", " CategoryNew", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent spaces, end").arg(prefix), 6, 6, "CategoryOld", "CategoryNew ", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent spaces, end, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryNew ", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, adjacent spaces, end, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryNew ", "CategoryNew", oldData, newData, QVector<int>() << 1 << 3);

        addTest(QString("%1, same category").arg(prefix), 6, 6, "CategoryOld", "CategoryOld", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryOld", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryOld", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, spaces, begin").arg(prefix), 6, 6, "CategoryOld", " CategoryOld", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, spaces, begin, entry-only").arg(prefix), 6, 0, "CategoryOld", " CategoryOld", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, spaces, begin, no entries").arg(prefix), 0, 6, "CategoryOld", " CategoryOld", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, spaces, end").arg(prefix), 6, 6, "CategoryOld", "CategoryOld ", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, spaces, end, entry-only").arg(prefix), 6, 0, "CategoryOld", "CategoryOld ", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, same category, spaces, end, no entries").arg(prefix), 0, 6, "CategoryOld", "CategoryOld ", "CategoryOld", oldData, newData, QVector<int>() << 1 << 3);

        addTest(QString("%1, merge categories").arg(prefix), 6, 6, "CategoryOld", "Category4", "Category4", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, from entry-only").arg(prefix), 6, 1, "CategoryOld", "Category0", "Category0", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, to entry-only").arg(prefix), 6, 2, "CategoryOld", "Category2", "Category2", oldData, newData, QVector<int>() << 1 << 2);
        addTest(QString("%1, merge categories, no entries").arg(prefix), 0, 6, "CategoryOld", "Category4", "Category4", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, spaces, begin").arg(prefix), 6, 6, "CategoryOld", " Category4", "Category4", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, spaces, begin, from entry-only").arg(prefix), 6, 1, "CategoryOld", " Category0", "Category0", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, spaces, begin, to entry-only").arg(prefix), 6, 2, "CategoryOld", " Category2", "Category2", oldData, newData, QVector<int>() << 1 << 2);
        addTest(QString("%1, merge categories, spaces, begin, no entries").arg(prefix), 0, 6, "CategoryOld", " Category4", "Category4", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, spaces, end").arg(prefix), 6, 6, "CategoryOld", "Category4 ", "Category4", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, spaces, end, from entry-only").arg(prefix), 6, 1, "CategoryOld", "Category0 ", "Category0", oldData, newData, QVector<int>() << 1 << 3);
        addTest(QString("%1, merge categories, spaces, end, to entry-only").arg(prefix), 6, 2, "CategoryOld", "Category2 ", "Category2", oldData, newData, QVector<int>() << 1 << 2);
        addTest(QString("%1, merge categories, spaces, end, no entries").arg(prefix), 0, 6, "CategoryOld", "Category4 ", "Category4", oldData, newData, QVector<int>() << 1 << 3);
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

void tst_DB::undoEntryInsert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy insertSpy(history, SIGNAL(dataInserted(TimeLogEntry)));
    QSignalSpy removeSpy(history, SIGNAL(dataRemoved(TimeLogEntry)));
    QSignalSpy undoCountSpy(history, SIGNAL(undoCountChanged(int)));

    if (initialEntries) {
        QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
        history->import(origData);
        QVERIFY(importSpy.wait());
    }

    QFETCH(int, index);
    TimeLogEntry entry;
    QFETCH(QDateTime, insertTime);
    entry.startTime = insertTime;
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    entry.uuid = QUuid::createUuid();
    history->insert(entry);
    QVERIFY(insertSpy.wait());
    QVERIFY(!updateSpy.isEmpty() || updateSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 1);
    QCOMPARE(history->undoCount(), 1);

    updateSpy.clear();
    undoCountSpy.clear();
    history->undo();
    QVERIFY(removeSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 0);
    QCOMPARE(history->undoCount(), 0);

    if (initialEntries < 1) {
        QVERIFY(updateSpy.isEmpty());
        return;
    }

    origData.insert(index, entry);

    checkFunction(checkRemove, removeSpy, updateSpy, origData, index);

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::undoEntryInsert_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");
    QTest::addColumn<QDateTime>("insertTime");

    QTest::newRow("empty db") << 0 << 0 << QDateTime::fromString("2015-11-01T15:00:00+0200", Qt::ISODate);
    QTest::newRow("1 entry, begin") << 1 << 0 << QDateTime::fromString("2015-11-01T10:30:00+0200", Qt::ISODate);
    QTest::newRow("1 entry, end") << 1 << 1 << QDateTime::fromString("2015-11-01T11:30:00+0200", Qt::ISODate);
    QTest::newRow("2 entries") << 2 << 2 << QDateTime::fromString("2015-11-01T11:30:00+0200", Qt::ISODate);
    QTest::newRow("6 entries") << 6 << 4 << QDateTime::fromString("2015-11-01T23:30:00+0200", Qt::ISODate);
}

void tst_DB::undoEntryRemove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy removeSpy(history, SIGNAL(dataRemoved(TimeLogEntry)));
    QSignalSpy insertSpy(history, SIGNAL(dataInserted(TimeLogEntry)));
    QSignalSpy undoCountSpy(history, SIGNAL(undoCountChanged(int)));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait());

    QFETCH(int, index);
    history->remove(origData.at(index));
    QVERIFY(removeSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 1);
    QCOMPARE(history->undoCount(), 1);

    if (initialEntries < 2) {
        QVERIFY(updateSpy.isEmpty());
        return;
    } else {
        QVERIFY(!updateSpy.isEmpty() || updateSpy.wait());
    }

    updateSpy.clear();
    undoCountSpy.clear();
    history->undo();
    QVERIFY(insertSpy.wait());
    QVERIFY(!updateSpy.isEmpty() || updateSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 0);
    QCOMPARE(history->undoCount(), 0);

    checkFunction(checkInsert, insertSpy, updateSpy, origData, index);

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::undoEntryRemove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries, first") << 2 << 0;
    QTest::newRow("2 entries, last") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 3;
}

void tst_DB::undoEntryEdit()
{
    QVector<TimeLogEntry> origData(defaultEntries());

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy undoCountSpy(history, SIGNAL(undoCountChanged(int)));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait());

    QFETCH(int, index);
    QFETCH(TimeLogEntry, newData);

    TimeLogEntry entry = origData.at(index);
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
    history->edit(entry, fields);
    QVERIFY(updateSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 1);
    QCOMPARE(history->undoCount(), 1);

    updateSpy.clear();
    undoCountSpy.clear();
    history->undo();
    QVERIFY(updateSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 0);
    QCOMPARE(history->undoCount(), 0);

    checkFunction(checkEdit, updateSpy, origData, fields, index);

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::undoEntryEdit_data()
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
}

void tst_DB::undoEntryMultiple()
{
    QVector<TimeLogEntry> origData(defaultEntries());

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy insertSpy(history, SIGNAL(dataInserted(TimeLogEntry)));
    QSignalSpy removeSpy(history, SIGNAL(dataRemoved(TimeLogEntry)));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy undoCountSpy(history, SIGNAL(undoCountChanged(int)));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait());

    // edit
    TimeLogEntry entry = origData.at(2);
    entry.category = "CategoryNew";
    history->edit(entry, TimeLogHistory::Category);
    QVERIFY(updateSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 1);
    QCOMPARE(history->undoCount(), 1);

    // insert
    updateSpy.clear();
    undoCountSpy.clear();
    entry.startTime = origData.constLast().startTime.addSecs(150);
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    entry.uuid = QUuid::createUuid();
    history->insert(entry);
    QVERIFY(insertSpy.wait());
    QVERIFY(!updateSpy.isEmpty() || updateSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 2);
    QCOMPARE(history->undoCount(), 2);

    // remove
    updateSpy.clear();
    undoCountSpy.clear();
    history->remove(origData.at(4));
    QVERIFY(removeSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 3);
    QCOMPARE(history->undoCount(), 3);

    // undo remove
    insertSpy.clear();
    updateSpy.clear();
    undoCountSpy.clear();
    history->undo();
    QVERIFY(insertSpy.wait());
    QVERIFY(!updateSpy.isEmpty() || updateSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 2);
    QCOMPARE(history->undoCount(), 2);

    QVERIFY(compareData(insertSpy.constFirst().at(0).value<TimeLogEntry>(), origData.at(4)));

    // undo insert
    removeSpy.clear();
    updateSpy.clear();
    undoCountSpy.clear();
    history->undo();
    QVERIFY(removeSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 1);
    QCOMPARE(history->undoCount(), 1);

    QVERIFY(compareData(removeSpy.constFirst().at(0).value<TimeLogEntry>(), entry));

    // undo edit
    updateSpy.clear();
    undoCountSpy.clear();
    history->undo();
    QVERIFY(updateSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 0);
    QCOMPARE(history->undoCount(), 0);

    checkFunction(checkEdit, updateSpy, origData, TimeLogHistory::Category, 2);

    checkFunction(checkDB, history, origData);

    checkFunction(checkHashes, history, false);
}

void tst_DB::undoCategoryAdd()
{
    QFETCH(int, initialEntries);
    QFETCH(int, initialCategories);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, initialCategories));

    QFETCH(QString, categoryName);
    QFETCH(QVariantMap, categoryData);
    TimeLogCategory category(QUuid::createUuid(), TimeLogCategoryData(categoryName, categoryData));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy categoriesSpy(history, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));
    QSignalSpy undoCountSpy(history, SIGNAL(undoCountChanged(int)));

    checkFunction(importSyncData, history, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    if (categoryName.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    } else if (categoryName != "CategoryNew" && initialEntries <= initialCategories) {
        QTest::ignoreMessage(QtCriticalMsg,
                             QRegularExpression(QString("Category '%1' already exists").arg(categoryName)));
    }
    categoriesSpy.clear();
    history->addCategory(category);
    if (categoryName.isEmpty()
        || (categoryName != "CategoryNew" && initialEntries <= initialCategories)) {
        QVERIFY(errorSpy.wait());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(categoriesSpy.isEmpty());
        QVERIFY(undoCountSpy.isEmpty());
        QCOMPARE(history->undoCount(), 0);
    } else {
        if (categoryName != "CategoryNew" && initialEntries > initialCategories) {
            QVERIFY(categoriesSpy.isEmpty());
        } else {
            QVERIFY(categoriesSpy.wait());
        }
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
        QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 1);
        QCOMPARE(history->undoCount(), 1);

        outdateSpy.clear();
        categoriesSpy.clear();
        undoCountSpy.clear();
        history->undo();
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
        if (categoryName != "CategoryNew" && initialEntries > initialCategories) {
            QVERIFY(categoriesSpy.isEmpty());
        } else {
            QVERIFY(!categoriesSpy.isEmpty() || categoriesSpy.wait());
        }
        QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
        QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 0);
        QCOMPARE(history->undoCount(), 0);
    }

    checkFunction(checkDB, history, origData);
    checkFunction(checkDB, history, origCategories);

    checkFunction(checkHashes, history, false);
}

void tst_DB::undoCategoryAdd_data()
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

void tst_DB::undoCategoryRemove()
{
    QFETCH(int, initialEntries);
    QFETCH(int, initialCategories);

    QVector<TimeLogEntry> origData(defaultEntries().mid(0, initialEntries));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, initialCategories));

    QFETCH(int, index);
    QString categoryName(index == -1 ? "" : defaultCategories().at(index).name);

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy categoriesSpy(history, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));
    QSignalSpy undoCountSpy(history, SIGNAL(undoCountChanged(int)));

    checkFunction(importSyncData, history, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    if (categoryName.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    } else if (qMax(initialCategories, initialEntries) <= index) {
        QTest::ignoreMessage(QtCriticalMsg,
                             QRegularExpression(QString("No such category: %1").arg(categoryName)));
    }
    categoriesSpy.clear();
    history->removeCategory(categoryName);
    if (categoryName.isEmpty() || qMax(initialCategories, initialEntries) <= index) {
        QVERIFY(errorSpy.wait());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(categoriesSpy.isEmpty());
    } else {
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
        if (initialEntries <= index) {  // Only deleting category without entries should emit signal
            QVERIFY(categoriesSpy.wait());
        } else {
            QVERIFY(categoriesSpy.isEmpty());
        }
        QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
        QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 1);
        QCOMPARE(history->undoCount(), 1);

        outdateSpy.clear();
        categoriesSpy.clear();
        undoCountSpy.clear();
        history->undo();
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
        if (initialEntries <= index) {  // Only deleting category without entries should emit signal
            QVERIFY(!categoriesSpy.isEmpty() || categoriesSpy.wait());
        } else {
            QVERIFY(categoriesSpy.isEmpty());
            if (initialCategories <= index) {
                // For entry-only category uuid generated on deletion, so get actual uuid from db
                QVector<TimeLogSyncDataEntry> syncEntryData;
                QVector<TimeLogSyncDataCategory> syncCategoryData;
                checkFunction(extractSyncData, history, syncEntryData, syncCategoryData);

                TimeLogCategory category(defaultCategories().at(index));
                auto it = std::find_if(syncCategoryData.cbegin(), syncCategoryData.cend(), [&category](const TimeLogSyncDataCategory &d) {
                    return d.category.name == category.name;
                });
                QVERIFY(it != syncCategoryData.cend());
                QVERIFY(!it->sync.isRemoved);
                category.uuid = it->category.uuid;
                origCategories.append(category);
            }
        }
        QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
        QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 0);
        QCOMPARE(history->undoCount(), 0);
    }

    checkFunction(checkDB, history, origData);
    checkFunction(checkDB, history, origCategories);

    checkFunction(checkHashes, history, false);
}

void tst_DB::undoCategoryRemove_data()
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

void tst_DB::undoCategoryEdit()
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

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy categoriesSpy(history, SIGNAL(categoriesChanged(QSharedPointer<TimeLogCategoryTreeNode>)));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy undoCountSpy(history, SIGNAL(undoCountChanged(int)));

    checkFunction(importSyncData, history, genSyncData(origData, defaultMTimes()),
                  genSyncData(origCategories, defaultMTimes()), 1);

    if (categoryNameNew.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression(QString("Empty category name")));
    }
    outdateSpy.clear();
    categoriesSpy.clear();
    updateSpy.clear();
    history->editCategory(categoryNameOld, category);
    if (categoryNameNew.isEmpty()) {
        QVERIFY(errorSpy.wait());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(updateSpy.isEmpty());
        QVERIFY(categoriesSpy.isEmpty());
        QVERIFY(undoCountSpy.isEmpty());
        QCOMPARE(history->undoCount(), 0);
    } else {
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
        if (categoryNameNew == categoryNameOld
            && (categoryDataNew.isEmpty() || categoryDataNew == categoryDataOld)
            && initialEntries > initialCategories) {
            QVERIFY(categoriesSpy.isEmpty());
        } else {
            QVERIFY(!categoriesSpy.isEmpty() || categoriesSpy.wait());
        }
        QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
        QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 1);
        QCOMPARE(history->undoCount(), 1);

        outdateSpy.clear();
        updateSpy.clear();
        categoriesSpy.clear();
        undoCountSpy.clear();
        history->undo();
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
        QVERIFY(!categoriesSpy.isEmpty() || categoriesSpy.wait());
        QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
        QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 0);
        QCOMPARE(history->undoCount(), 0);

        QVector<int> updateIndices;
        for (int index: indices) {
            if (categoryNameNew != categoryNameOld && index < initialEntries) {
                updateIndices.append(index);
            }
        }
        while (updateSpy.size() < updateIndices.size()) {
            QVERIFY(updateSpy.wait());
        }
        QCOMPARE(updateSpy.size(), updateIndices.size());

        for (int i = 0; i < updateIndices.size(); i++) {
            QVector<TimeLogEntry> updateData = updateSpy.at(i).at(0).value<QVector<TimeLogEntry> >();
            QVector<TimeLogHistory::Fields> updateFields = updateSpy.at(i).at(1).value<QVector<TimeLogHistory::Fields> >();
            QCOMPARE(updateData.size(), 1);
            QCOMPARE(updateData.constFirst().category, origData.at(updateIndices.at(i)).category);
            QCOMPARE(updateFields.constFirst(), TimeLogHistory::Category);
        }
    }

    checkFunction(checkHashes, history, false);
}

void tst_DB::undoCategoryEdit_data()
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

void tst_DB::hashes()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy(history, SIGNAL(dataOutdated()));

    QSignalSpy historyHashesSpy(history, SIGNAL(hashesAvailable(QMap<QDateTime,QByteArray>)));

    checkFunction(importSyncData, history, origSyncEntries, origSyncCategories, 1);

    checkFunction(checkHashesUpdated, history, false);

    historyHashesSpy.clear();
    history->getHashes();
    QVERIFY(historyHashesSpy.wait());
    QVERIFY(historyErrorSpy.isEmpty());
    QVERIFY(historyOutdateSpy.isEmpty());

    QMap<QDateTime,QByteArray> hashes = historyHashesSpy.constFirst().at(0).value<QMap<QDateTime,QByteArray> >();

    checkFunction(checkHashesUpdated, hashes, true);
    checkFunction(compareHashes, hashes, calcHashes(origSyncEntries, origSyncCategories));

    checkFunction(checkDB, history, origEntries);
    checkFunction(checkDB, history, origCategories);
    checkFunction(checkDB, history, origSyncEntries, origSyncCategories);
}

void tst_DB::hashes_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");

    auto addTest = [](int entries, int categories)
    {
        QTest::newRow(QString("%1 entries, %2 categories").arg(entries).arg(categories).toLocal8Bit())
                << entries << categories;
    };

    auto addTestSet = [&addTest](int entries)
    {
        addTest(entries, 0);
        addTest(entries, 1);
        addTest(entries, 2);
        addTest(entries, 4);
        addTest(entries, 6);
    };

    addTestSet(0);
    addTestSet(1);
    addTestSet(2);
    addTestSet(4);
    addTestSet(6);
}

void tst_DB::hashesUpdate()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy(history, SIGNAL(dataOutdated()));

    QSignalSpy historyUpdateHashesSpy(history, SIGNAL(hashesUpdated()));

    checkFunction(importSyncData, history, origSyncEntries, origSyncCategories, 1);

    checkFunction(checkHashesUpdated, history, false);

    historyUpdateHashesSpy.clear();
    history->updateHashes();
    QVERIFY(historyUpdateHashesSpy.wait());
    QVERIFY(historyErrorSpy.isEmpty());
    QVERIFY(historyOutdateSpy.isEmpty());

    checkFunction(checkHashesUpdated, history, true);
    checkFunction(checkHashes, history, calcHashes(origSyncEntries, origSyncCategories));

    checkFunction(checkDB, history, origEntries);
    checkFunction(checkDB, history, origCategories);
    checkFunction(checkDB, history, origSyncEntries, origSyncCategories);
}

void tst_DB::hashesUpdate_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");

    auto addTest = [](int entries, int categories)
    {
        QTest::newRow(QString("%1 entries, %2 categories").arg(entries).arg(categories).toLocal8Bit())
                << entries << categories;
    };

    auto addTestSet = [&addTest](int entries)
    {
        addTest(entries, 0);
        addTest(entries, 1);
        addTest(entries, 2);
        addTest(entries, 4);
        addTest(entries, 6);
    };

    addTestSet(0);
    addTestSet(1);
    addTestSet(2);
    addTestSet(4);
    addTestSet(6);
}

void tst_DB::hashesOld()
{
    QFETCH(int, entriesCount);
    QFETCH(int, categoriesCount);

    QVector<TimeLogEntry> origEntries(defaultEntries().mid(0, entriesCount));
    QVector<TimeLogCategory> origCategories(defaultCategories().mid(0, categoriesCount));

    QVector<TimeLogSyncDataEntry> origSyncEntries(genSyncData(origEntries, defaultMTimes()));
    QVector<TimeLogSyncDataCategory> origSyncCategories(genSyncData(origCategories, defaultMTimes()));

    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));

    QSignalSpy historyOutdateSpy(history, SIGNAL(dataOutdated()));

    QSignalSpy historyUpdateHashesSpy(history, SIGNAL(hashesUpdated()));

    checkFunction(importSyncData, history, origSyncEntries, origSyncCategories, 1);

    QMap<QDateTime,QByteArray> hashes;

    checkFunction(extractHashes, history, hashes, false);
    checkFunction(checkHashes, history, calcHashes(origSyncEntries, origSyncCategories));

    QFETCH(QVector<TimeLogSyncDataEntry>, newEntries);
    QFETCH(QVector<TimeLogSyncDataCategory>, newCategories);

    checkFunction(importSyncData, history, newEntries, newCategories, 1);

    QDateTime periodStart;
    if (!newEntries.isEmpty()) {
        updateDataSet(origEntries, newEntries.constFirst().entry);
        updateDataSet(origSyncEntries, newEntries.constFirst());
        periodStart = monthStart(newEntries.constFirst().sync.mTime);
    }
    if (!newCategories.isEmpty()) {
        updateDataSet(origCategories, newCategories.constFirst().category);
        updateDataSet(origSyncCategories, newCategories.constFirst());
        periodStart = monthStart(newCategories.constFirst().sync.mTime);
    }
    hashes.insert(periodStart, calcHashes(origSyncEntries, origSyncCategories).value(periodStart));

    historyUpdateHashesSpy.clear();
    history->updateHashes();
    QVERIFY(historyUpdateHashesSpy.wait());
    QVERIFY(historyErrorSpy.isEmpty());
    QVERIFY(historyOutdateSpy.isEmpty());

    checkFunction(checkHashes, history, hashes);

    checkFunction(checkDB, history, origEntries);
    checkFunction(checkDB, history, origCategories);
    checkFunction(checkDB, history, origSyncEntries, origSyncCategories);
}

void tst_DB::hashesOld_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("categoriesCount");

    QTest::addColumn<QVector<TimeLogSyncDataEntry> >("newEntries");
    QTest::addColumn<QVector<TimeLogSyncDataCategory> >("newCategories");

    auto addInsertTest = [](int size, int index, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.startTime = defaultEntries().at(index).startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        entry.uuid = QUuid::createUuid();
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries, insert %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << 0
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category;
        category.name = "CategoryNew";
        category.uuid = QUuid::createUuid();
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories, add %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << 0 << size
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories, add %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << size
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addInsertTests = [&addInsertTest](int size, int index, const QDateTime &maxMonth)
    {
        addInsertTest(size, index, monthStart(maxMonth), "0");
        addInsertTest(size, index, monthStart(maxMonth).addMSecs(-1), "-1 ms");
        addInsertTest(size, index, monthStart(maxMonth).addMSecs(1), "+1 ms");
        addInsertTest(size, index, monthStart(maxMonth).addMSecs(-999), "-999 ms");
        addInsertTest(size, index, monthStart(maxMonth).addMSecs(999), "+999 ms");
        addInsertTest(size, index, monthStart(maxMonth).addSecs(-1), "-1 s");
        addInsertTest(size, index, monthStart(maxMonth).addSecs(1), "+1 s");
        addInsertTest(size, index, monthStart(maxMonth).addMSecs(-1001), "-1001 ms");
        addInsertTest(size, index, monthStart(maxMonth).addMSecs(1001), "+1001 ms");
        addInsertTest(size, index, monthStart(maxMonth).addDays(10), "+10 days");
        addInsertTest(size, index, monthStart(maxMonth).addMonths(2), "+2 months");
        addInsertTest(size, index, monthStart(maxMonth).addYears(1), "+1 year");
    };

    addInsertTests(1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addInsertTests(2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addInsertTests(4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addInsertTests(6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));


    auto addEditTest = [](int size, int index, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        TimeLogSyncDataEntry syncEntry;

        entry = defaultEntries().at(index);
        entry.startTime = entry.startTime.addSecs(100);
        syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries, edit start %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << 0
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        entry = defaultEntries().at(index);
        entry.category = "CategoryNew";
        syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries, edit category %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << 0
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        entry = defaultEntries().at(index);
        entry.comment = "Test comment";
        syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries, edit comment %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << 0
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        entry = defaultEntries().at(index);
        entry.startTime = entry.startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries, edit all %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << 0
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category = defaultCategories().at(index);
        category.name = "CategoryNew";
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories, edit %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << 0 << size
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories, edit %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << size
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addEditTests = [&addEditTest](int size, int index, const QDateTime &maxMonth)
    {
        addEditTest(size, index, monthStart(maxMonth), "0");
        addEditTest(size, index, monthStart(maxMonth).addMSecs(-1), "-1 ms");
        addEditTest(size, index, monthStart(maxMonth).addMSecs(1), "+1 ms");
        addEditTest(size, index, monthStart(maxMonth).addMSecs(-999), "-999 ms");
        addEditTest(size, index, monthStart(maxMonth).addMSecs(999), "+999 ms");
        addEditTest(size, index, monthStart(maxMonth).addSecs(-1), "-1 s");
        addEditTest(size, index, monthStart(maxMonth).addSecs(1), "+1 s");
        addEditTest(size, index, monthStart(maxMonth).addMSecs(-1001), "-1001 ms");
        addEditTest(size, index, monthStart(maxMonth).addMSecs(1001), "+1001 ms");
        addEditTest(size, index, monthStart(maxMonth).addDays(10), "+10 days");
        addEditTest(size, index, monthStart(maxMonth).addMonths(2), "+2 months");
        addEditTest(size, index, monthStart(maxMonth).addYears(1), "+1 year");
    };

    addEditTests(1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addEditTests(2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addEditTests(4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addEditTests(6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    auto addRemoveTest = [](int size, int index, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.uuid = defaultEntries().at(index).uuid;
        TimeLogSyncDataEntry syncEntry = TimeLogSyncDataEntry(entry, mTime);

        QTest::newRow(QString("%1 entries, remove %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << 0
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << QVector<TimeLogSyncDataCategory>();

        TimeLogCategory category;
        category.uuid = defaultCategories().at(index).uuid;
        TimeLogSyncDataCategory syncCategory = TimeLogSyncDataCategory(category, mTime);

        QTest::newRow(QString("%1 categories, remove %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << 0 << size
                << QVector<TimeLogSyncDataEntry>()
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);

        QTest::newRow(QString("%1 entries, %1 categories, remove %2 %3").arg(size).arg(index).arg(info).toLocal8Bit())
                << size << size
                << (QVector<TimeLogSyncDataEntry>() << syncEntry)
                << (QVector<TimeLogSyncDataCategory>() << syncCategory);
    };

    auto addRemoveTests = [&addRemoveTest](int size, int index, const QDateTime &maxMonth)
    {
        addRemoveTest(size, index, monthStart(maxMonth), "0");
        addRemoveTest(size, index, monthStart(maxMonth).addMSecs(-1), "-1 ms");
        addRemoveTest(size, index, monthStart(maxMonth).addMSecs(1), "+1 ms");
        addRemoveTest(size, index, monthStart(maxMonth).addMSecs(-999), "-999 ms");
        addRemoveTest(size, index, monthStart(maxMonth).addMSecs(999), "+999 ms");
        addRemoveTest(size, index, monthStart(maxMonth).addSecs(-1), "-1 s");
        addRemoveTest(size, index, monthStart(maxMonth).addSecs(1), "+1 s");
        addRemoveTest(size, index, monthStart(maxMonth).addMSecs(-1001), "-1001 ms");
        addRemoveTest(size, index, monthStart(maxMonth).addMSecs(1001), "+1001 ms");
        addRemoveTest(size, index, monthStart(maxMonth).addDays(10), "+10 days");
        addRemoveTest(size, index, monthStart(maxMonth).addMonths(2), "+2 months");
        addRemoveTest(size, index, monthStart(maxMonth).addYears(1), "+1 year");
    };

    addRemoveTests(1, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addRemoveTests(2, 0, QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC));

    addRemoveTests(4, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));

    addRemoveTests(6, 0, QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC));
}

QTEST_MAIN(tst_DB)
#include "tst_db.moc"
