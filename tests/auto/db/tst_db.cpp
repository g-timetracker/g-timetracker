#include <QtTest/QtTest>

#include <QTemporaryDir>

#include "TimeLogHistory.h"
#include "TimeLogEntry.h"

#define checkFunction(func, ...) do {   \
    func(__VA_ARGS__);                  \
    if (QTest::currentTestFailed()) {   \
        QFAIL("Subtest failed");        \
    }                                   \
} while (0)

QTemporaryDir *dataDir = Q_NULLPTR;
TimeLogHistory *history = Q_NULLPTR;

QVector<TimeLogEntry> defaultData;

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
    void insert();
    void insert_data();
    void insertConflict();
    void insertConflict_data();
    void remove();
    void remove_data();
    void edit();
    void edit_data();
    void editConflict();
    void editConflict_data();
    void renameCategory();
    void renameCategory_data();
    void undoInsert();
    void undoInsert_data();
    void undoRemove();
    void undoRemove_data();
    void undoEdit();
    void undoEdit_data();
    void undoRenameCategory();
    void undoRenameCategory_data();
    void undoMultiple();

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

    initDefaultData();
}

void tst_DB::import()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(defaultData.mid(0, entriesCount));

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
}

void tst_DB::import_data()
{
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("empty db") << 0;
    QTest::newRow("1 entry") << 1;
    QTest::newRow("2 entries") << 2;
    QTest::newRow("6 entries") << 6;
}

void tst_DB::insert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

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
}

void tst_DB::insert_data()
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

void tst_DB::insertConflict()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

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
}

void tst_DB::insertConflict_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 4;
}

void tst_DB::remove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

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
}

void tst_DB::remove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries, first") << 2 << 0;
    QTest::newRow("2 entries, last") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 3;
}

void tst_DB::edit()
{
    QVector<TimeLogEntry> origData(defaultData);

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
}

void tst_DB::edit_data()
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

void tst_DB::editConflict()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

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
}

void tst_DB::editConflict_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");
    QTest::addColumn<TimeLogEntry>("newData");

    int index = 1;
    TimeLogEntry entry = defaultData.at(index);
    entry.startTime = defaultData.at(0).startTime;
    QTest::newRow("2 entries, first") << 2 << index << entry;

    index = 0;
    entry = defaultData.at(index);
    entry.startTime = defaultData.at(1).startTime;
    QTest::newRow("2 entries, last") << 2 << index << entry;

    index = 3;
    entry = defaultData.at(index);
    entry.startTime = defaultData.at(4).startTime;
    QTest::newRow("6 entries, middle") << 6 << index << entry;

    index = 5;
    entry = defaultData.at(index);
    entry.startTime = defaultData.at(2).startTime;
    entry.category = "CategoryNew";
    QTest::newRow("6 entries, category") << 6 << index << entry;

    index = 1;
    entry = defaultData.at(index);
    entry.startTime = defaultData.at(3).startTime;
    entry.comment = "Test comment";
    QTest::newRow("6 entries, comment") << 6 << index << entry;

    index = 2;
    entry = defaultData.at(index);
    entry.startTime = defaultData.at(0).startTime;
    entry.category = "CategoryNew";
    entry.comment = "Test comment";
    QTest::newRow("6 entries, all") << 6 << index << entry;
}

void tst_DB::renameCategory()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));

    QFETCH(QString, categoryOld);
    QFETCH(QString, categoryNew);
    QFETCH(QVector<int>, indexes);
    foreach (int index, indexes) {
        origData[index].category = categoryOld;
    }

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait());

    if (categoryNew == categoryOld) {
        QTest::ignoreMessage(QtWarningMsg, QString("Same category name: \"%1\"").arg(categoryOld).toLocal8Bit());
    } else if (categoryNew.isEmpty()) {
        QTest::ignoreMessage(QtCriticalMsg, "Empty category name");
    }
    history->editCategory(categoryOld, categoryNew);
    if (categoryNew == categoryOld) {
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(outdateSpy.isEmpty());
    } else if (categoryNew.isEmpty()) {
        QVERIFY(errorSpy.wait());
        QVERIFY(outdateSpy.isEmpty());
    } else {
        QVERIFY(outdateSpy.wait());
        QVERIFY(errorSpy.isEmpty());

        foreach (int index, indexes) {
            origData[index].category = categoryNew;
        }
    }

    checkFunction(checkDB, history, origData);
}

void tst_DB::renameCategory_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<QString>("categoryOld");
    QTest::addColumn<QString>("categoryNew");
    QTest::addColumn<QVector<int> >("indexes");

    QTest::newRow("same category") << 6 << "CategoryOld" << "CategoryOld" << (QVector<int>() << 1 << 3);
    QTest::newRow("empty category") << 6 << "CategoryOld" << "" << (QVector<int>() << 1 << 3);
    QTest::newRow("different category") << 6 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 1 << 3);
    QTest::newRow("1 entry") << 1 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 0);
    QTest::newRow("2 entries, first") << 2 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 0);
    QTest::newRow("2 entries, last") << 2 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 1);
    QTest::newRow("all items") << 6 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 0 << 1 << 2 << 3 << 4 << 5);
}

void tst_DB::undoInsert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

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
}

void tst_DB::undoInsert_data()
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

void tst_DB::undoRemove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

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
}

void tst_DB::undoRemove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("index");

    QTest::newRow("1 entry") << 1 << 0;
    QTest::newRow("2 entries, first") << 2 << 0;
    QTest::newRow("2 entries, last") << 2 << 1;
    QTest::newRow("6 entries") << 6 << 3;
}

void tst_DB::undoEdit()
{
    QVector<TimeLogEntry> origData(defaultData);

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
}

void tst_DB::undoEdit_data()
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

void tst_DB::undoRenameCategory()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(defaultData.mid(0, initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    QSignalSpy undoCountSpy(history, SIGNAL(undoCountChanged(int)));

    QFETCH(QString, categoryOld);
    QFETCH(QString, categoryNew);
    QFETCH(QVector<int>, indexes);
    foreach (int index, indexes) {
        origData[index].category = categoryOld;
    }

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait());

    history->editCategory(categoryOld, categoryNew);
    QVERIFY(outdateSpy.wait());
    QVERIFY(!undoCountSpy.isEmpty() || undoCountSpy.wait());
    QCOMPARE(undoCountSpy.constFirst().at(0).value<int>(), 1);
    QCOMPARE(history->undoCount(), 1);

    updateSpy.clear();
    outdateSpy.clear();
    history->undo();
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());

    while (updateSpy.size() < indexes.size()) {
        QVERIFY(updateSpy.wait());
    }

    for (int i = 0; i < indexes.size(); i++) {
        QVector<TimeLogEntry> updateData = updateSpy.at(i).at(0).value<QVector<TimeLogEntry> >();
        QVector<TimeLogHistory::Fields> updateFields = updateSpy.at(i).at(1).value<QVector<TimeLogHistory::Fields> >();
        QCOMPARE(updateData.size(), 1);
        QCOMPARE(updateData.constFirst().category, origData.at(indexes.at(i)).category);
        QCOMPARE(updateFields.constFirst(), TimeLogHistory::Category);
    }
}

void tst_DB::undoRenameCategory_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<QString>("categoryOld");
    QTest::addColumn<QString>("categoryNew");
    QTest::addColumn<QVector<int> >("indexes");

    QTest::newRow("different category") << 6 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 1 << 3);
    QTest::newRow("1 entry") << 1 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 0);
    QTest::newRow("2 entries, first") << 2 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 0);
    QTest::newRow("2 entries, last") << 2 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 1);
    QTest::newRow("all items") << 6 << "CategoryOld" << "CategoryNew" << (QVector<int>() << 0 << 1 << 2 << 3 << 4 << 5);
}

void tst_DB::undoMultiple()
{
    QVector<TimeLogEntry> origData(defaultData);

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
}

void tst_DB::checkInsert(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index)
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

void tst_DB::checkRemove(QSignalSpy &actionSpy, QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, int index)
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

void tst_DB::checkEdit(QSignalSpy &updateSpy, QVector<TimeLogEntry> &origData, TimeLogHistory::Fields fields, int index)
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

void tst_DB::checkDB(TimeLogHistory *history, const QVector<TimeLogEntry> &data) const
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

void tst_DB::initDefaultData()
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

void tst_DB::dumpData(const QVector<TimeLogEntry> &data) const
{
    for (int i = 0; i < data.size(); i++) {
        qDebug() << i << data.at(i) << endl;
    }
}

bool tst_DB::checkData(const QVector<TimeLogEntry> &data) const
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

bool tst_DB::compareData(const TimeLogEntry &t1, const TimeLogEntry &t2) const
{
    if (t1.uuid != t2.uuid || t1.startTime != t2.startTime || t1.category != t2.category
        || t1.comment != t2.comment) {
        qCritical() << "Data does not match" << endl << t1 << endl << t2 << endl;
        return false;
    }

    return true;
}

bool tst_DB::compareData(const QVector<TimeLogEntry> &d1, const QVector<TimeLogEntry> &d2) const
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

QTEST_MAIN(tst_DB)
#include "tst_db.moc"
