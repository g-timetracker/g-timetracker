#include <QtTest/QtTest>

#include <QTemporaryDir>

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

const QString syncFileNamePattern = QString("^%1$").arg(fileNamePattern);
const QRegularExpression syncFileNameRegexp(syncFileNamePattern);

const QString packFileNamePattern = QString("^%1.pack$").arg(fileNamePattern);
const QRegularExpression packFileNameRegexp(packFileNamePattern);

void importSyncData(TimeLogHistory *history, DataSyncer *syncer, QTemporaryDir *syncDir, const QVector<TimeLogSyncData> &data, int portionSize, bool noPack = true, const QDateTime &syncTime = QDateTime::currentDateTimeUtc())
{
    QSignalSpy historyErrorSpy(history, SIGNAL(error(QString)));
    QSignalSpy historyOutdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy historySyncSpy(history, SIGNAL(dataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)));

    QSignalSpy syncSpy(syncer, SIGNAL(synced()));
    QSignalSpy syncErrorSpy(syncer, SIGNAL(error(QString)));

    syncer->setNoPack(true);

    int importedSize = 0;
    while (importedSize < data.size()) {
        int importSize = qMin(portionSize, data.size() - importedSize);
        QVector<TimeLogSyncData> syncPortion = data.mid(importedSize, importSize);

        historySyncSpy.clear();
        history->sync(syncPortion, QVector<TimeLogSyncData>());
        QVERIFY(historySyncSpy.wait());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());

        importedSize += importSize;
        QCOMPARE(history->size(), importedSize);

        syncSpy.clear();
        syncer->sync(QUrl::fromLocalFile(syncDir->path()), syncTime);
        QVERIFY(syncSpy.wait());
        QVERIFY(syncErrorSpy.isEmpty());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());
    }

    if (data.isEmpty()) {
        syncSpy.clear();
        syncer->sync(QUrl::fromLocalFile(syncDir->path()), syncTime);
        QVERIFY(syncSpy.wait());
        QVERIFY(syncErrorSpy.isEmpty());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());
    }

    syncer->setNoPack(false);

    if (!noPack) {
        syncSpy.clear();
        syncer->sync(QUrl::fromLocalFile(syncDir->path()), syncTime);
        QVERIFY(syncSpy.wait());
        QVERIFY(syncErrorSpy.isEmpty());
        QVERIFY(historyErrorSpy.isEmpty());
        QVERIFY(historyOutdateSpy.isEmpty());
    }

    checkFunction(checkDB, history, data);
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

QVector<QDateTime> mTimeList(const QFileInfoList &infoList, const QRegularExpression &regexp)
{
    QVector<QDateTime> result;

    result.reserve(infoList.size());
    for (const QFileInfo &entry: infoList) {
        QString fileName = entry.fileName();
        QRegularExpressionMatch match(regexp.match(fileName));
        if (match.hasMatch()) {
            result.append(QDateTime::fromMSecsSinceEpoch(match.captured("mTime").toLongLong()));
        }
    }

    return result;
}

QVector<QDateTime> syncMTimeList(const QString &path)
{
    return mTimeList(buildFileList(path, false), syncFileNameRegexp);
}

void checkSyncFolder(const QString &path, const QVector<TimeLogSyncData> &data, int syncPortion)
{
    QVector<QDateTime> mTimes;

    for (int i = syncPortion - 1; i < data.size(); i += syncPortion) {
        mTimes.append(data.at(i).mTime);
    }

    QCOMPARE(syncMTimeList(path), mTimes);
}

void checkPackHashes(const QString &path, const QString &file)
{
    QScopedPointer<TimeLogHistory> pack(new TimeLogHistory());
    QVERIFY(pack->init(path, file));
    checkHashesUpdated(pack.data(), true);
}

void checkPackFolder(const QString &path, const QVector<TimeLogSyncData> &data, int syncPortion,
                     const QDateTime &maxPacked, const QVector<QDateTime> &notPacked = QVector<QDateTime>())
{
    QVector<QDateTime> mTimes(notPacked);

    QDateTime packMTime;

    for (int stepSize = qMin(syncPortion, data.size()), i = stepSize - 1;
         i >= 0 && i < data.size();
         stepSize = qMin(syncPortion, data.size() - i), i += stepSize) {
        if (data.at(i).mTime > maxPacked) {
            mTimes.append(data.at(i).mTime);
        }
    }
    std::sort(mTimes.begin(), mTimes.end());

    for (const TimeLogSyncData &entry: data) {
        if (entry.mTime <= maxPacked) {
            packMTime = entry.mTime;
        } else {
            break;
        }
    }

    QCOMPARE(syncMTimeList(path), mTimes);

    QFileInfoList packList(buildFileList(path, false, QStringList() << "*.pack"));
    for (const QFileInfo &fileInfo: packList) {
        checkPackHashes(path, fileInfo.fileName());
    }

    QCOMPARE(mTimeList(packList, packFileNameRegexp),
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
    qRegisterMetaType<QVector<TimeLogSyncData> >();
    qRegisterMetaType<QSharedPointer<TimeLogCategory> >();
    qRegisterMetaType<QMap<QDateTime,QByteArray> >();

    oldCategoryFilter = QLoggingCategory::installFilter(nullptr);
    QLoggingCategory::installFilter(syncerCategoryFilter);
}

void tst_SyncPack::cleanupTestCase()
{
}

void tst_SyncPack::pack()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(defaultData().mid(0, entriesCount));
    QVector<TimeLogSyncData> origSyncData(genSyncData(origData, defaultMTimes()));

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

    checkFunction(importSyncData, history1, syncer1, syncDir1, origSyncData, syncPortion, true);

    checkFunction(checkSyncFolder, QDir(dataDir1->path()).filePath("sync"), origSyncData, syncPortion);

    // Pack
    syncSpy1.clear();
    syncer1->pack(syncDir1->path(), syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncData, syncPortion,
                  monthStart(syncStart).addMSecs(-1));

    // Sync 2 [in]
    syncer2->sync(QUrl::fromLocalFile(syncDir1->path()), syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history2, origSyncData);
    checkFunction(checkPackFolder, QDir(dataDir2->path()).filePath("sync"), origSyncData, syncPortion,
                  monthStart(syncStart).addMSecs(-1));

    // Sync 1 [in]
    syncer1->sync(QUrl::fromLocalFile(syncDir1->path()), syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history1, origSyncData);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncData, syncPortion,
                  monthStart(syncStart).addMSecs(-1));
}

void tst_SyncPack::pack_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("syncPortion");
    QTest::addColumn<QDateTime>("syncStart");

    QTest::newRow("empty, pack 0") << 0 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("1 entry, pack 0") << 1 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("1 entry, pack 1") << 1 << 1 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("1 entry, pack 1 -1 ms") << 1 << 1 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("1 entry, pack 1 +1 ms") << 1 << 1 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("2 entries by 1, pack 0") << 2 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("2 entries by 1, pack 2") << 2 << 1 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("2 entries by 2, pack 2") << 2 << 2 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("2 entries by 1, pack 2 -1 ms") << 2 << 1 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("2 entries by 1, pack 2 +1 ms") << 2 << 1 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("2 entries by 2, pack 2 -1 ms") << 2 << 2 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("2 entries by 2, pack 2 +1 ms") << 2 << 2 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("4 entries by 1, pack 0") << 4 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 1, pack 2") << 4 << 1 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 1, pack 4") << 4 << 1 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 2, pack 4") << 4 << 2 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 4, pack 4") << 4 << 4 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 1, pack 4 -1 ms") << 4 << 1 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("4 entries by 1, pack 4 +1 ms") << 4 << 1 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("4 entries by 4, pack 4 -1 ms") << 4 << 4 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("4 entries by 4, pack 4 +1 ms") << 4 << 4 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("6 entries by 1, pack 0") << 6 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 1, pack 2") << 6 << 1 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 1, pack 6") << 6 << 1 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 2, pack 6") << 6 << 2 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 3, pack 6") << 6 << 3 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 6, pack 6") << 6 << 3 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 1, pack 6 -1 ms") << 6 << 1 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("6 entries by 1, pack 6 +1 ms") << 6 << 1 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("6 entries by 6, pack 6 -1 ms") << 6 << 6 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("6 entries by 6, pack 6 +1 ms") << 6 << 6 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(1);
}

void tst_SyncPack::syncPack()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(defaultData().mid(0, entriesCount));
    QVector<TimeLogSyncData> origSyncData(genSyncData(origData, defaultMTimes()));

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

    checkFunction(importSyncData, history1, syncer1, syncDir1, origSyncData, syncPortion, false, syncStart);

    // Sync 1 [out]
    syncer1->sync(QUrl::fromLocalFile(syncDir1->path()), syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history1, origSyncData);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncData, syncPortion,
                  monthStart(syncStart).addMSecs(-1));

    // Sync 2 [in]
    syncer2->sync(QUrl::fromLocalFile(syncDir1->path()), syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history2, origSyncData);
    checkFunction(checkPackFolder, QDir(dataDir2->path()).filePath("sync"), origSyncData, syncPortion,
                  monthStart(syncStart).addMSecs(-1));

    // Sync 1 [in]
    syncer1->sync(QUrl::fromLocalFile(syncDir1->path()), syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history1, origSyncData);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncData, syncPortion,
                  monthStart(syncStart).addMSecs(-1));
}

void tst_SyncPack::syncPack_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("syncPortion");
    QTest::addColumn<QDateTime>("syncStart");

    QTest::newRow("empty, pack 0") << 0 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("1 entry, pack 0") << 1 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("1 entry, pack 1") << 1 << 1 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("1 entry, pack 1 -1 ms") << 1 << 1 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("1 entry, pack 1 +1 ms") << 1 << 1 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("2 entries by 1, pack 0") << 2 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("2 entries by 1, pack 2") << 2 << 1 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("2 entries by 2, pack 2") << 2 << 2 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("2 entries by 1, pack 2 -1 ms") << 2 << 1 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("2 entries by 1, pack 2 +1 ms") << 2 << 1 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("2 entries by 2, pack 2 -1 ms") << 2 << 2 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("2 entries by 2, pack 2 +1 ms") << 2 << 2 << QDateTime(QDate(2015, 12, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("4 entries by 1, pack 0") << 4 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 1, pack 2") << 4 << 1 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 1, pack 4") << 4 << 1 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 2, pack 4") << 4 << 2 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 4, pack 4") << 4 << 4 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("4 entries by 1, pack 4 -1 ms") << 4 << 1 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("4 entries by 1, pack 4 +1 ms") << 4 << 1 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("4 entries by 4, pack 4 -1 ms") << 4 << 4 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("4 entries by 4, pack 4 +1 ms") << 4 << 4 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("6 entries by 1, pack 0") << 6 << 1 << QDateTime(QDate(2015, 11, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 1, pack 2") << 6 << 1 << QDateTime(QDate(2015, 12, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 1, pack 6") << 6 << 1 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 2, pack 6") << 6 << 2 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 3, pack 6") << 6 << 3 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 6, pack 6") << 6 << 3 << QDateTime(QDate(2016, 01, 10), QTime(), Qt::UTC);
    QTest::newRow("6 entries by 1, pack 6 -1 ms") << 6 << 1 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("6 entries by 1, pack 6 +1 ms") << 6 << 1 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(1);
    QTest::newRow("6 entries by 6, pack 6 -1 ms") << 6 << 6 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(-1);
    QTest::newRow("6 entries by 6, pack 6 +1 ms") << 6 << 6 << QDateTime(QDate(2016, 01, 01), QTime(), Qt::UTC).addMSecs(1);
}

void tst_SyncPack::unpacked()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(defaultData().mid(0, entriesCount));
    QVector<TimeLogSyncData> origSyncData(genSyncData(origData, defaultMTimes()));

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

    checkFunction(importSyncData, history1, syncer1, syncDir1, origSyncData, syncPortion, false, syncStart);

    QFETCH(TimeLogSyncData, newData);

    checkFunction(importSyncData, history2, QVector<TimeLogSyncData>() << newData, syncPortion);

    QVector<TimeLogEntry> updatedData(origData);
    QVector<TimeLogSyncData> updatedSyncData(origSyncData);
    updateDataSet(updatedData, static_cast<TimeLogEntry>(newData));
    updateDataSet(updatedSyncData, newData);

    // Sync 2 [out]
    syncer2->setNoPack(true);
    syncSpy2.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir2->path()), syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in-out]
    syncSpy1.clear();
    syncer1->sync(QUrl::fromLocalFile(syncDir2->path()), syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    QDateTime maxPackedDate = monthStart(syncStart).addMSecs(-1);

    checkFunction(checkDB, history1, updatedData);
    checkFunction(checkDB, history1, updatedSyncData);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncData, syncPortion,
                  maxPackedDate, QVector<QDateTime>() << newData.mTime);

    // Sync 2 [in]
    syncer2->setNoPack(false);
    syncSpy2.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir2->path()), syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, updatedData);
    checkFunction(checkDB, history2, updatedSyncData);
    checkFunction(checkPackFolder,
                  QDir(dataDir2->path()).filePath("sync"),
                  maxPackedDate < newData.mTime ? origSyncData : updatedSyncData,
                  syncPortion,
                  maxPackedDate,
                  maxPackedDate < newData.mTime ? QVector<QDateTime>() << newData.mTime : QVector<QDateTime>());

    // Sync 3 [in]
    syncer3->sync(QUrl::fromLocalFile(syncDir2->path()), syncStart);
    QVERIFY(syncSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history3, updatedData);
    checkFunction(checkDB, history3, updatedSyncData);
    checkFunction(checkPackFolder,
                  QDir(dataDir2->path()).filePath("sync"),
                  maxPackedDate < newData.mTime ? origSyncData : updatedSyncData,
                  syncPortion,
                  maxPackedDate,
                  maxPackedDate < newData.mTime ? QVector<QDateTime>() << newData.mTime : QVector<QDateTime>());
}

void tst_SyncPack::unpacked_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("syncPortion");
    QTest::addColumn<QDateTime>("syncStart");
    QTest::addColumn<TimeLogSyncData>("newData");

    auto addInsertTest = [](int size, int syncPortion, int packCount, int index,
                            const QDateTime &syncStart, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.startTime = defaultData().at(index).startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        entry.uuid = QUuid::createUuid();
        TimeLogSyncData syncData = TimeLogSyncData(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, insert %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << syncPortion << syncStart << syncData;
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
        TimeLogEntry entry = defaultData().at(index);
        entry.startTime = entry.startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        entry.uuid = defaultData().at(index).uuid;
        TimeLogSyncData syncData = TimeLogSyncData(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, edit %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << syncPortion << syncStart << syncData;
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
        entry.uuid = defaultData().at(index).uuid;
        TimeLogSyncData syncData = TimeLogSyncData(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, remove %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << syncPortion << syncStart << syncData;
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

    QVector<TimeLogEntry> origData(defaultData().mid(0, entriesCount));
    QVector<TimeLogSyncData> origSyncData(genSyncData(origData, defaultMTimes()));

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

    checkFunction(importSyncData, history1, syncer1, syncDir1, origSyncData, syncPortion, false, syncStart);

    QFETCH(TimeLogSyncData, newData);

    checkFunction(importSyncData, history2, QVector<TimeLogSyncData>() << newData, syncPortion);

    updateDataSet(origData, static_cast<TimeLogEntry>(newData));
    updateDataSet(origSyncData, newData);

    // Sync 2 [out]
    syncer2->setNoPack(false);
    syncSpy2.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir2->path()), monthStart(newData.mTime.addMonths(1)));
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    // Sync 1 [in-out]
    syncSpy1.clear();
    syncer1->sync(QUrl::fromLocalFile(syncDir2->path()), syncStart);
    QVERIFY(syncSpy1.wait());
    QVERIFY(syncErrorSpy1.isEmpty());
    QVERIFY(historyErrorSpy1.isEmpty());
    QVERIFY(historyOutdateSpy1.isEmpty());

    QDateTime maxPackedDate = qMax(monthStart(syncStart).addMSecs(-1), monthStart(newData.mTime.addMonths(1)).addMSecs(-1));

    checkFunction(checkDB, history1, origData);
    checkFunction(checkDB, history1, origSyncData);
    checkFunction(checkPackFolder, QDir(dataDir1->path()).filePath("sync"), origSyncData, syncPortion,
                  maxPackedDate);

    // Sync 2 [in]
    syncSpy2.clear();
    syncer2->sync(QUrl::fromLocalFile(syncDir2->path()), syncStart);
    QVERIFY(syncSpy2.wait());
    QVERIFY(syncErrorSpy2.isEmpty());
    QVERIFY(historyErrorSpy2.isEmpty());
    QVERIFY(historyOutdateSpy2.isEmpty());

    checkFunction(checkDB, history2, origData);
    checkFunction(checkDB, history2, origSyncData);
    checkFunction(checkPackFolder, QDir(dataDir2->path()).filePath("sync"), origSyncData, syncPortion,
                  maxPackedDate);

    // Sync 3 [in]
    syncer3->sync(QUrl::fromLocalFile(syncDir2->path()), syncStart);
    QVERIFY(syncSpy3.wait());
    QVERIFY(syncErrorSpy3.isEmpty());
    QVERIFY(historyErrorSpy3.isEmpty());
    QVERIFY(historyOutdateSpy3.isEmpty());

    checkFunction(checkDB, history3, origData);
    checkFunction(checkDB, history3, origSyncData);
    checkFunction(checkPackFolder, QDir(dataDir2->path()).filePath("sync"), origSyncData, syncPortion,
                  maxPackedDate);
}

void tst_SyncPack::twoPacks_data()
{
    QTest::addColumn<int>("entriesCount");
    QTest::addColumn<int>("syncPortion");
    QTest::addColumn<QDateTime>("syncStart");
    QTest::addColumn<TimeLogSyncData>("newData");

    auto addInsertTest = [](int size, int syncPortion, int packCount, int index,
                            const QDateTime &syncStart, const QDateTime &mTime, const QString &info)
    {
        TimeLogEntry entry;
        entry.startTime = defaultData().at(index).startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        entry.uuid = QUuid::createUuid();
        TimeLogSyncData syncData = TimeLogSyncData(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, insert %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << syncPortion << syncStart << syncData;
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
        TimeLogEntry entry = defaultData().at(index);
        entry.startTime = entry.startTime.addSecs(100);
        entry.category = "CategoryNew";
        entry.comment = "Test comment";
        entry.uuid = defaultData().at(index).uuid;
        TimeLogSyncData syncData = TimeLogSyncData(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, edit %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << syncPortion << syncStart << syncData;
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
        entry.uuid = defaultData().at(index).uuid;
        TimeLogSyncData syncData = TimeLogSyncData(entry, mTime);

        QTest::newRow(QString("%1 entries by %2, pack %3, remove %4 %5").arg(size).arg(syncPortion)
                      .arg(packCount).arg(index).arg(info).toLocal8Bit())
                << size << syncPortion << syncStart << syncData;
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
