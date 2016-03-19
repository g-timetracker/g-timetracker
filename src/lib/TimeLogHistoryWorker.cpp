#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QDataStream>
#include <QCryptographicHash>

#include <QLoggingCategory>

#include "TimeLogHistoryWorker.h"
#include "TimeLogCategory.h"

Q_LOGGING_CATEGORY(HISTORY_WORKER_CATEGORY, "TimeLogHistoryWorker", QtInfoMsg)

const QString categorySplitPattern("\\s*>\\s*");

const int maxUndoSize(10);

const QString selectFields("SELECT uuid, start, category, comment, duration,"
                           " ifnull((SELECT start FROM timelog WHERE start < result.start ORDER BY start DESC LIMIT 1), 0)"
                           " FROM timelog AS result");

TimeLogHistoryWorker::TimeLogHistoryWorker(QObject *parent) :
    QObject(parent),
    m_isInitialized(false),
    m_size(0),
    m_categorySplitRegexp(categorySplitPattern),
    m_insertQuery(Q_NULLPTR),
    m_removeQuery(Q_NULLPTR),
    m_notifyInsertQuery(Q_NULLPTR),
    m_notifyRemoveQuery(Q_NULLPTR),
    m_notifyEditQuery(Q_NULLPTR),
    m_notifyEditStartQuery(Q_NULLPTR),
    m_syncAffectedQuery(Q_NULLPTR),
    m_entryQuery(Q_NULLPTR)
{

}

TimeLogHistoryWorker::~TimeLogHistoryWorker()
{
    delete m_insertQuery;
    delete m_removeQuery;
    delete m_notifyInsertQuery;
    delete m_notifyRemoveQuery;
    delete m_notifyEditQuery;
    delete m_notifyEditStartQuery;
    delete m_syncAffectedQuery;
    delete m_entryQuery;

    QSqlDatabase::database(m_connectionName).close();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool TimeLogHistoryWorker::init(const QString &dataPath, const QString &filePath)
{
    QString dirPath(QString("%1%2")
                    .arg(!dataPath.isEmpty() ? dataPath
                                             : QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                    .arg(filePath.isEmpty() ? "/timelog" : ""));

    if (!(QDir().mkpath(dirPath))) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to create directory for db";
        return false;
    }

    QString dbPath(QString("%1/%2").arg(dirPath).arg(filePath.isEmpty() ? "db.sqlite" : filePath));

    m_connectionName = QString("timelog_%1_%2").arg(qHash(dbPath)).arg(QDateTime::currentMSecsSinceEpoch());
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to open db:" << db.lastError().text();
        return false;
    }

    if (!setupTable()) {
        return false;
    }

    if (!setupTriggers()) {
        return false;
    }

    if (!updateSize()) {
        return false;
    }

    if (!updateCategories()) {
        return false;
    }

    m_isInitialized = true;

    return true;
}

qlonglong TimeLogHistoryWorker::size() const
{
    return m_size;
}

QSharedPointer<TimeLogCategory> TimeLogHistoryWorker::categories() const
{
    return m_categoryTree;
}

void TimeLogHistoryWorker::insert(const TimeLogEntry &data)
{
    Q_ASSERT(m_isInitialized);

    Undo undo;
    undo.type = Undo::Insert;
    undo.data.append(data);
    pushUndo(undo);

    insertEntry(data);
}

void TimeLogHistoryWorker::import(const QVector<TimeLogEntry> &data)
{
    Q_ASSERT(m_isInitialized);

    if (insertData(data)) {
        emit dataImported(data);
    } else {
        processFail();
    }

    return;
}

void TimeLogHistoryWorker::remove(const TimeLogEntry &data)
{
    Q_ASSERT(m_isInitialized);

    TimeLogEntry entry = getEntry(data.uuid);
    Undo undo;
    undo.type = Undo::Remove;
    undo.data.append(entry);
    pushUndo(undo);

    removeEntry(data);
}

void TimeLogHistoryWorker::edit(const TimeLogEntry &data, TimeLogHistory::Fields fields)
{
    Q_ASSERT(m_isInitialized);

    TimeLogEntry entry = getEntry(data.uuid);
    Undo undo;
    undo.type = Undo::Edit;
    undo.data.append(entry);
    undo.fields.append(fields);
    pushUndo(undo);

    editEntry(data, fields);
}

void TimeLogHistoryWorker::editCategory(QString oldName, QString newName)
{
    Q_ASSERT(m_isInitialized);

    if (newName.isEmpty()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Empty category name";
        emit error("Empty category name");
        return;
    } else if (oldName == newName) {
        qCWarning(HISTORY_WORKER_CATEGORY) << "Same category name:" << newName;
        return;
    }

    QVector<TimeLogEntry> entries = getEntries(oldName);
    Undo undo;
    undo.type = Undo::EditCategory;
    undo.data.swap(entries);
    undo.fields.insert(0, undo.data.size(), TimeLogHistory::Category);
    pushUndo(undo);

    if (editCategoryData(oldName, newName)) {
        emit dataOutdated();    // TODO: more precise update signal
    } else {
        processFail();
    }
}

void TimeLogHistoryWorker::sync(const QVector<TimeLogSyncData> &updatedData, const QVector<TimeLogSyncData> &removedData)
{
    Q_ASSERT(m_isInitialized);

    QVector<TimeLogSyncData> removedNew;
    QVector<TimeLogSyncData> removedOld;
    QVector<TimeLogSyncData> insertedNew;
    QVector<TimeLogSyncData> insertedOld;
    QVector<TimeLogSyncData> updatedNew;
    QVector<TimeLogSyncData> updatedOld;

    foreach (const TimeLogSyncData &entry, removedData) {
        QVector<TimeLogSyncData> affected = getSyncAffected(entry.uuid);
        if (!affected.isEmpty() && affected.constFirst().mTime >= entry.mTime) {
            continue;
        }

        removedNew.append(entry);
        removedOld.append(affected.isEmpty() ? TimeLogSyncData() : affected.constFirst());
    }

    foreach (const TimeLogSyncData &entry, updatedData) {
        QVector<TimeLogSyncData> affected = getSyncAffected(entry.uuid);
        if (!affected.isEmpty() && affected.constFirst().mTime >= entry.mTime) {
            continue;
        }

        if (affected.isEmpty() || !affected.constFirst().isValid()) {
            insertedNew.append(entry);
            insertedOld.append(affected.isEmpty() ? TimeLogSyncData() : affected.constFirst());
        } else {
            updatedNew.append(entry);
            updatedOld.append(affected.constFirst());
        }
    }

    emit syncStatsAvailable(removedOld, removedNew, insertedOld, insertedNew, updatedOld, updatedNew);

    QVector<TimeLogSyncData> removedMerged(removedNew.size());
    for (int i = 0; i < removedMerged.size(); i++) {
        removedMerged[i] = removedOld.at(i);
        removedMerged[i].uuid = removedNew.at(i).uuid;
        removedMerged[i].mTime = removedNew.at(i).mTime;
    }

    if (syncData(removedMerged, insertedNew, updatedNew, updatedOld)) {
        emit dataSynced(updatedData, removedData);
    }
}

void TimeLogHistoryWorker::updateHashes()
{
    updateDataHashes(getDataHashes());

    emit hashesUpdated();
}

void TimeLogHistoryWorker::undo()
{
    if (!m_undoStack.size()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Empty undo stack";
        return;
    }

    Undo undo = m_undoStack.pop();
    switch (undo.type) {
    case Undo::Insert:
        removeEntry(undo.data.constFirst());
        break;
    case Undo::Remove:
        insertEntry(undo.data.constFirst());
        break;
    case Undo::Edit:
        editEntry(undo.data.constFirst(), undo.fields.constFirst());
        break;
    case Undo::EditCategory:
        editEntries(undo.data, undo.fields);
        break;
    }

    emit undoCountChanged(m_undoStack.size());
}

void TimeLogHistoryWorker::getHistoryBetween(qlonglong id, const QDateTime &begin, const QDateTime &end, const QString &category) const
{
    Q_ASSERT(m_isInitialized);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString = QString("%1 WHERE (start BETWEEN ? AND ?) %2 ORDER BY start ASC")
                                  .arg(selectFields)
                                  .arg(category.isEmpty() ? "" : "AND category=?");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        emit historyRequestCompleted(QVector<TimeLogEntry>(), id);
        return;
    }
    query.addBindValue(begin.toTime_t());
    query.addBindValue(end.toTime_t());
    if (!category.isEmpty()) {
        query.addBindValue(category);
    }

    emit historyRequestCompleted(getHistory(query), id);
}

void TimeLogHistoryWorker::getHistoryAfter(qlonglong id, const uint limit, const QDateTime &from) const
{
    Q_ASSERT(m_isInitialized);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString = QString("%1 WHERE start > ? ORDER BY start ASC LIMIT ?").arg(selectFields);
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        emit historyRequestCompleted(QVector<TimeLogEntry>(), id);
        return;
    }
    query.addBindValue(from.toTime_t());
    query.addBindValue(limit);

    emit historyRequestCompleted(getHistory(query), id);
}

void TimeLogHistoryWorker::getHistoryBefore(qlonglong id, const uint limit, const QDateTime &until) const
{
    Q_ASSERT(m_isInitialized);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString = QString("%1 WHERE start < ? ORDER BY start DESC LIMIT ?").arg(selectFields);
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        emit historyRequestCompleted(QVector<TimeLogEntry>(), id);
        return;
    }
    query.addBindValue(until.toTime_t());
    query.addBindValue(limit);

    QVector<TimeLogEntry> result = getHistory(query);
    if (!result.isEmpty()) {
        std::reverse(result.begin(), result.end());
    }
    emit historyRequestCompleted(result, id);
}

void TimeLogHistoryWorker::getStats(const QDateTime &begin, const QDateTime &end, const QString &category, const QString &separator) const
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString = QString("WITH result AS ( "
                                  "    SELECT rtrim(substr(category, 1, ifnull(%1, length(category)))) as category, CASE "
                                  "        WHEN duration!=-1 THEN duration "
                                  "        ELSE (SELECT strftime('%s','now')) - (SELECT start FROM timelog ORDER BY start DESC LIMIT 1) "
                                  "        END AS duration "
                                  "    FROM timelog "
                                  "    WHERE (start BETWEEN :sBegin AND :sEnd) %2 "
                                  ") "
                                  "SELECT category, SUM(duration) FROM result "
                                  " GROUP BY category "
                                  " ORDER BY category ASC")
            .arg(category.isEmpty() ? "nullif(instr(category, :separator) - 1, -1)"
                                    : "nullif(instr(substr(category, nullif(instr(substr(category, length(:category) + 1), :separator), 0) + 1 + length(:category)), :separator), 0) + length(:category)")
            .arg(category.isEmpty() ? "" : "AND category LIKE :category || '%'");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return;
    }
    query.bindValue(":sBegin", begin.toTime_t());
    query.bindValue(":sEnd", end.toTime_t());
    query.bindValue(":separator", separator);
    if (!category.isEmpty()) {
        query.bindValue(":category", category);
    }

    emit statsDataAvailable(getStats(query), end);
}

void TimeLogHistoryWorker::getSyncData(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    Q_ASSERT(m_isInitialized);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString where = QString(mBegin.isValid() ? "mtime > :mBegin" : "")
                    + QString(mBegin.isValid() && mEnd.isValid() ? " AND " : "")
                    + QString(mEnd.isValid() ? "mtime <= :mEnd" : "");
    if (!where.isEmpty()) {
        where = "WHERE (" + where + ")";
    }
    QString queryString = QString("WITH result AS ( "
                                  "    SELECT uuid, start, category, comment, mtime FROM timelog %1 "
                                  "UNION ALL "
                                  "    SELECT uuid, NULL, NULL, NULL, mtime FROM removed %1 "
                                  ") "
                                  "SELECT * FROM result ORDER BY mtime ASC").arg(where);
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return;
    }
    if (mBegin.isValid()) {
        query.bindValue(":mBegin", mBegin.toMSecsSinceEpoch());
    }
    if (mEnd.isValid()) {
        query.bindValue(":mEnd", mEnd.toMSecsSinceEpoch());
    }

    emit syncDataAvailable(getSyncData(query), mEnd);
}

void TimeLogHistoryWorker::getSyncDataSize(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    Q_ASSERT(m_isInitialized);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("SELECT count(*) FROM ( "
                        "    SELECT mtime FROM timelog "
                        "    WHERE (mtime > :mBegin AND mtime <= :mEnd) "
                        "UNION "
                        "    SELECT mtime FROM removed "
                        "    WHERE (mtime > :mBegin AND mtime <= :mEnd) "
                        ")");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return;
    }
    query.bindValue(":mBegin", mBegin.toMSecsSinceEpoch());
    query.bindValue(":mEnd", mEnd.toMSecsSinceEpoch());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return;
    }

    query.next();
    qlonglong result = query.value(0).toLongLong();
    query.finish();

    emit syncDataSizeAvailable(result, mBegin, mEnd);
}

void TimeLogHistoryWorker::getHashes(const QDateTime &maxDate, bool noUpdate)
{
    QMap<QDateTime, QByteArray> hashes(getDataHashes(maxDate));
    if (!noUpdate && std::any_of(hashes.cbegin(), hashes.cend(), [](const QByteArray &hash) {
        return hash.isEmpty();
    })) {
        updateDataHashes(hashes);
        hashes = getDataHashes(maxDate);
    }

    emit hashesAvailable(hashes);
}

bool TimeLogHistoryWorker::setupTable()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("CREATE TABLE IF NOT EXISTS timelog"
                        " (uuid BLOB UNIQUE, start INTEGER PRIMARY KEY, category TEXT, comment TEXT,"
                        " duration INTEGER, mtime INTEGER);");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE INDEX IF NOT EXISTS timelog_mtime_index ON timelog (mtime);";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE TABLE IF NOT EXISTS removed (uuid BLOB PRIMARY KEY, mtime INTEGER) WITHOUT ROWID;";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE INDEX IF NOT EXISTS removed_mtime_index ON removed (mtime);";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE TABLE IF NOT EXISTS hashes (start INTEGER PRIMARY KEY, hash BLOB);";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    return true;
}

bool TimeLogHistoryWorker::setupTriggers()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString;

    queryString = "CREATE TRIGGER IF NOT EXISTS check_insert_timelog BEFORE INSERT ON timelog "
                  "BEGIN "
                  "    SELECT mtime, "
                  "        CASE WHEN NEW.mtime < mtime "
                  "            THEN RAISE(IGNORE) "
                  "        END "
                  "    FROM removed WHERE uuid=NEW.uuid; "
                  "END;";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS insert_timelog AFTER INSERT ON timelog "
                  "BEGIN "
                  "    UPDATE timelog SET duration=(NEW.start - start) "
                  "    WHERE start=( "
                  "        SELECT start FROM timelog WHERE start < NEW.start ORDER BY start DESC LIMIT 1 "
                  "    ); "
                  "    UPDATE timelog SET duration=IFNULL( "
                  "        ( SELECT start FROM timelog WHERE start > NEW.start ORDER BY start ASC LIMIT 1 ) - NEW.start, "
                  "        -1 "
                  "    ) WHERE start=NEW.start; "
                  "    DELETE FROM removed WHERE uuid=NEW.uuid; "
                  "    INSERT OR REPLACE INTO hashes (start, hash) VALUES(strftime('%s', NEW.mtime/1000, 'unixepoch', 'start of month'), NULL); "
                  "END;";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS delete_timelog AFTER DELETE ON timelog "
                  "BEGIN "
                  "    UPDATE timelog SET duration=IFNULL( "
                  "        ( SELECT start FROM timelog WHERE start > OLD.start ORDER BY start ASC LIMIT 1 ) - start, "
                  "        -1 "
                  "    ) WHERE start=( "
                  "        SELECT start FROM timelog WHERE start < OLD.start ORDER BY start DESC LIMIT 1 "
                  "    ); "
                  "END;";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS check_update_timelog BEFORE UPDATE ON timelog "
                  "BEGIN "
                  "    SELECT "
                  "        CASE WHEN NEW.mtime < OLD.mtime "
                  "            THEN RAISE(IGNORE) "
                  "        END; "
                  "END;";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS update_timelog AFTER UPDATE OF start ON timelog "
                  "BEGIN "
                  "    UPDATE timelog SET duration=(NEW.start - start) "
                  "    WHERE start=( "
                  "        SELECT start FROM timelog WHERE start < NEW.start ORDER BY start DESC LIMIT 1 "
                  "    ); "
                  "    UPDATE timelog SET duration=IFNULL( "
                  "        ( SELECT start FROM timelog WHERE start > OLD.start ORDER BY start ASC LIMIT 1 ) - start,"
                  "        -1"
                  "    ) WHERE start=NULLIF( "  // If previous item not changed, do not update it's duration twice
                  "        ( SELECT start FROM timelog WHERE start < OLD.start ORDER BY start DESC LIMIT 1 ), "
                  "        ( SELECT start FROM timelog WHERE start < NEW.start ORDER BY start DESC LIMIT 1 ) "
                  "    ); "
                  "    UPDATE timelog SET duration=IFNULL( "
                  "        ( SELECT start FROM timelog WHERE start > NEW.start ORDER BY start ASC LIMIT 1 ) - NEW.start, "
                  "        -1 "
                  "    ) WHERE start=NEW.start; "
                  "    INSERT OR REPLACE INTO hashes (start, hash) VALUES(strftime('%s', NEW.mtime/1000, 'unixepoch', 'start of month'), NULL); "
                  "END;";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS check_insert_removed BEFORE INSERT ON removed "
                  "BEGIN "
                  "    SELECT mtime, "
                  "        CASE WHEN NEW.mtime < mtime "
                  "            THEN RAISE(IGNORE) "
                  "        END "
                  "    FROM removed WHERE uuid=NEW.uuid; "
                  "END;";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS insert_removed AFTER INSERT ON removed "
                  "BEGIN "
                  "    DELETE FROM timelog WHERE uuid=NEW.uuid; "
                  "    INSERT OR REPLACE INTO hashes (start, hash) VALUES(strftime('%s', NEW.mtime/1000, 'unixepoch', 'start of month'), NULL); "
                  "END;";
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        return false;
    }

    return true;
}

void TimeLogHistoryWorker::setSize(qlonglong size)
{
    if (m_size == size) {
        return;
    }

    m_size = size;

    emit sizeChanged(m_size);
}

void TimeLogHistoryWorker::removeFromCategories(QString category)
{
    if (!m_categorySet.contains(category)) {
        return;
    }

    m_categorySet.remove(category);
    m_categoryTree = parseCategories(m_categorySet);

    emit categoriesChanged(m_categoryTree);
}

void TimeLogHistoryWorker::addToCategories(QString category)
{
    if (m_categorySet.contains(category)) {
        return;
    }

    m_categorySet.insert(category);
    m_categoryTree = parseCategories(m_categorySet);

    emit categoriesChanged(m_categoryTree);
}

void TimeLogHistoryWorker::processFail()
{
    m_undoStack.clear();
    emit undoCountChanged(0);

    emit dataOutdated();
}

void TimeLogHistoryWorker::insertEntry(const TimeLogEntry &data)
{
    if (insertData(data)) {
        emit dataInserted(data);
        notifyInsertUpdates(data);
    } else {
        processFail();
    }
}

void TimeLogHistoryWorker::removeEntry(const TimeLogEntry &data)
{
    if (removeData(data)) {
        emit dataRemoved(data);
        notifyRemoveUpdates(data);
    } else {
        processFail();
    }
}

bool TimeLogHistoryWorker::editEntry(const TimeLogEntry &data, TimeLogHistory::Fields fields)
{
    QDateTime oldStart;
    if (fields == TimeLogHistory::NoFields) {
        qCWarning(HISTORY_WORKER_CATEGORY) << "No fields specified";
        return false;
    } else if (fields & TimeLogHistory::StartTime) {
        TimeLogEntry oldData = getEntry(data.uuid);
        if (!oldData.isValid()) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Item to update not found:\n"
                                                << data.startTime << data.category << data.uuid;
            processFail();
            return false;
        }
        oldStart = oldData.startTime;
    }

    if (editData(data, fields)) {
        notifyEditUpdates(data, fields, oldStart);
    }  else {
        processFail();
        return false;
    }

    return true;
}

void TimeLogHistoryWorker::editEntries(const QVector<TimeLogEntry> &data, const QVector<TimeLogHistory::Fields> &fields)
{
    for (int i = 0; i < data.size(); i++) {
        if (!editEntry(data.at(i), fields.at(i))) {
            break;
        }
    }
}

bool TimeLogHistoryWorker::insertData(const QVector<TimeLogEntry> &data)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.transaction()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to start transaction:" << db.lastError().text();
        emit error(db.lastError().text());
        return false;
    }

    foreach (const TimeLogEntry &entry, data) {
        if (!insertData(entry)) {
            if (!db.rollback()) {
                qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to rollback transaction:" << db.lastError().text();
                emit error(db.lastError().text());
            }

            return false;
        }
    }

    if (!db.commit()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to commit transaction:" << db.lastError().text();
        emit error(db.lastError().text());
        if (!db.rollback()) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to rollback transaction:" << db.lastError().text();
            emit error(db.lastError().text());
        }
        return false;
    }

    return true;
}

bool TimeLogHistoryWorker::insertData(const TimeLogSyncData &data)
{
    Q_ASSERT(data.isValid());

    if (!m_insertQuery) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString = QString("INSERT INTO timelog (uuid, start, category, comment, mtime)"
                                      " VALUES (:uuid, :start, :category, :comment, :mtime);");
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(query->lastError().text());
            delete query;
            return false;
        }

        m_insertQuery = query;
    }

    m_insertQuery->bindValue(":uuid", data.uuid.toRfc4122());
    m_insertQuery->bindValue(":start", data.startTime.toTime_t());
    m_insertQuery->bindValue(":category", data.category);
    m_insertQuery->bindValue(":comment", data.comment);
    m_insertQuery->bindValue(":mtime", data.mTime.isValid() ? data.mTime.toMSecsSinceEpoch()
                                            : QDateTime::currentMSecsSinceEpoch());

    if (!m_insertQuery->exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:"
                                            << m_insertQuery->lastError().text()
                                            << m_insertQuery->executedQuery()
                                            << m_insertQuery->boundValues();
        emit error(m_insertQuery->lastError().text());
        return false;
    }

    setSize(m_size + m_insertQuery->numRowsAffected());
    addToCategories(data.category);

    return true;
}

bool TimeLogHistoryWorker::removeData(const TimeLogSyncData &data)
{
    Q_ASSERT(!data.uuid.isNull());

    if (!m_removeQuery) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString("INSERT OR REPLACE INTO removed (uuid, mtime) VALUES(:uuid,:mtime);");
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(query->lastError().text());
            delete query;
            return false;
        }

        m_removeQuery = query;
    }

    m_removeQuery->bindValue(":uuid", data.uuid.toRfc4122());
    m_removeQuery->bindValue(":mtime", data.mTime.isValid() ? data.mTime.toMSecsSinceEpoch()
                                                            : QDateTime::currentMSecsSinceEpoch());

    if (!m_removeQuery->exec()) {
        qCWarning(HISTORY_WORKER_CATEGORY) << "Fail to execute query:"
                                           << m_removeQuery->lastError().text()
                                           << m_removeQuery->executedQuery()
                                           << m_removeQuery->boundValues();
        emit error(m_removeQuery->lastError().text());
        return false;
    }

    setSize(m_size - m_removeQuery->numRowsAffected());

    return true;
}

bool TimeLogHistoryWorker::editData(const TimeLogSyncData &data, TimeLogHistory::Fields fields)
{
    Q_ASSERT(data.isValid());
    Q_ASSERT(fields != TimeLogHistory::NoFields);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QStringList fieldNames;
    if (fields & TimeLogHistory::StartTime) {
        fieldNames.append("start=?");
    }
    if (fields & TimeLogHistory::Category) {
        fieldNames.append("category=?");
    }
    if (fields & TimeLogHistory::Comment) {
        fieldNames.append("comment=?");
    }
    QString queryString = QString("UPDATE timelog SET %1, mtime=?"
                                  " WHERE uuid=?;").arg(fieldNames.join(", "));
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return false;
    }
    if (fields & TimeLogHistory::StartTime) {
        query.addBindValue(data.startTime.toTime_t());
    }
    if (fields & TimeLogHistory::Category) {
        query.addBindValue(data.category);
    }
    if (fields & TimeLogHistory::Comment) {
        query.addBindValue(data.comment);
    }
    query.addBindValue(data.mTime.isValid() ? data.mTime.toMSecsSinceEpoch()
                                            : QDateTime::currentMSecsSinceEpoch());
    query.addBindValue(data.uuid.toRfc4122());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return false;
    }

    if (fields & TimeLogHistory::Category) {
        addToCategories(data.category);
    }

    return true;
}

bool TimeLogHistoryWorker::editCategoryData(QString oldName, QString newName)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("SELECT count(*) FROM timelog WHERE category=?");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return false;
    }
    query.addBindValue(oldName);

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return false;
    }

    query.next();
    bool hasOldCategoryItems = query.value(0).toULongLong() > 0;
    query.finish();

    if (!hasOldCategoryItems) {
        removeFromCategories(oldName);
        return false;
    }

    queryString = QString("UPDATE timelog SET category=?, mtime=? WHERE category=?;");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return false;
    }
    query.addBindValue(newName);
    query.addBindValue(QDateTime::currentMSecsSinceEpoch());
    query.addBindValue(oldName);

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return false;
    }

    if (!updateCategories()) {
        return false;
    }

    return true;
}

bool TimeLogHistoryWorker::syncData(const QVector<TimeLogSyncData> &removed, const QVector<TimeLogSyncData> &inserted, const QVector<TimeLogSyncData> &updatedNew, const QVector<TimeLogSyncData> &updatedOld)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.transaction()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to start transaction:" << db.lastError().text();
        emit error(db.lastError().text());
        return false;
    }

    foreach (const TimeLogSyncData &entry, removed) {
        if (!removeData(entry)) {
            if (!db.rollback()) {
                qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to rollback transaction:" << db.lastError().text();
                emit error(db.lastError().text());
            }

            return false;
        }
    }

    foreach (const TimeLogSyncData &entry, inserted) {
        if (!insertData(entry)) {
            if (!db.rollback()) {
                qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to rollback transaction:" << db.lastError().text();
                emit error(db.lastError().text());
            }

            return false;
        }
    }

    foreach (const TimeLogSyncData &entry, updatedNew) {
        if (!editData(entry, TimeLogHistory::AllFieldsMask)) {
            if (!db.rollback()) {
                qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to rollback transaction:" << db.lastError().text();
                emit error(db.lastError().text());
            }

            return false;
        }
    }

    if (!db.commit()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to commit transaction:" << db.lastError().text();
        emit error(db.lastError().text());
        if (!db.rollback()) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to rollback transaction:" << db.lastError().text();
            emit error(db.lastError().text());
        }
        return false;
    }

    foreach (const TimeLogEntry &entry, removed) {
        if (entry.isValid()) {
            emit dataRemoved(entry);
        }
    }
    foreach (const TimeLogEntry &entry, removed) {
        if (entry.isValid()) {
            notifyRemoveUpdates(entry);
        }
    }
    foreach (const TimeLogEntry &entry, inserted) {
        emit dataInserted(entry);
    }
    foreach (const TimeLogEntry &entry, inserted) {
        notifyInsertUpdates(entry);
    }
    for (int i = 0; i < updatedNew.size(); i++) {
        const TimeLogSyncData &newField = updatedNew.at(i);
        const TimeLogSyncData &oldField = updatedOld.at(i);
        TimeLogHistory::Fields fields(TimeLogHistory::NoFields);
        if (newField.startTime != oldField.startTime) {
            fields |= TimeLogHistory::StartTime;
        }
        if (newField.category != oldField.category) {
            fields |= TimeLogHistory::Category;
        }
        if (newField.comment != oldField.comment) {
            fields |= TimeLogHistory::Comment;
        }
        notifyEditUpdates(updatedNew.at(i), fields, oldField.startTime);
    }

    return true;
}

void TimeLogHistoryWorker::updateDataHashes(const QMap<QDateTime, QByteArray> &hashes)
{
    for (auto it = hashes.cbegin(); it != hashes.cend(); it++) {
        if (it.value().isEmpty()) {
            qCDebug(HISTORY_WORKER_CATEGORY) << "Updating data hash for period start" << it.key();
            QByteArray hash = calcHash(it.key(), it.key().addMonths(1).addMSecs(-1));
            if (hash.isEmpty()) {
                if (!removeHash(it.key())) {
                    return;
                }
            } else {
                if (!writeHash(it.key(), hash)) {
                    return;
                }
            }
        }
    }
}

bool TimeLogHistoryWorker::writeHash(const QDateTime &start, const QByteArray &hash)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("UPDATE hashes SET hash=? WHERE start=?;");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return false;
    }
    query.addBindValue(hash);
    query.addBindValue(start.toTime_t());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return false;
    }

    return true;
}

bool TimeLogHistoryWorker::removeHash(const QDateTime &start)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("DELETE FROM hashes WHERE start=?;");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return false;
    }
    query.addBindValue(start.toTime_t());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return false;
    }

    return true;
}

QVector<TimeLogEntry> TimeLogHistoryWorker::getHistory(QSqlQuery &query) const
{
    QVector<TimeLogEntry> result;

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return result;
    }

    while (query.next()) {
        TimeLogEntry data;
        data.uuid = QUuid::fromRfc4122(query.value(0).toByteArray());
        data.startTime = QDateTime::fromTime_t(query.value(1).toUInt(), Qt::UTC);
        data.category = query.value(2).toString();
        data.comment = query.value(3).toString();
        data.durationTime = query.value(4).toInt();
        data.precedingStart = QDateTime::fromTime_t(query.value(5).toUInt(), Qt::UTC);

        result.append(data);
    }

    query.finish();

    return result;
}

QVector<TimeLogStats> TimeLogHistoryWorker::getStats(QSqlQuery &query) const
{
    QVector<TimeLogStats> result;

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return result;
    }

    while (query.next()) {
        TimeLogStats data;
        data.category = query.value(0).toString();
        data.durationTime = query.value(1).toInt();

        result.append(data);
    }

    query.finish();

    return result;
}

QVector<TimeLogSyncData> TimeLogHistoryWorker::getSyncData(QSqlQuery &query) const
{
    QVector<TimeLogSyncData> result;

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return result;
    }

    while (query.next()) {
        TimeLogSyncData data;
        data.uuid = QUuid::fromRfc4122(query.value(0).toByteArray());
        if (!query.isNull(1)) { // Removed item shouldn't has valid start time
            data.startTime = QDateTime::fromTime_t(query.value(1).toUInt(), Qt::UTC);
        }
        data.category = query.value(2).toString();
        data.comment = query.value(3).toString();
        data.mTime = QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong(), Qt::UTC);

        result.append(data);
    }

    query.finish();

    return result;
}

TimeLogEntry TimeLogHistoryWorker::getEntry(const QUuid &uuid)
{
    TimeLogEntry entry;

    if (!m_entryQuery) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString = QString("%1 WHERE uuid=:uuid").arg(selectFields);
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(query->lastError().text());
            delete query;
            return entry;
        }

        m_entryQuery = query;
    }

    m_entryQuery->bindValue(":uuid", uuid.toRfc4122());

    QVector<TimeLogEntry> data = getHistory(*m_entryQuery);
    if (!data.empty()) {
        entry = data.first();
    }
    return entry;
}

QVector<TimeLogEntry> TimeLogHistoryWorker::getEntries(const QString &category) const
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString = QString("%1 WHERE category=?").arg(selectFields);
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return QVector<TimeLogEntry>();
    }
    query.addBindValue(category);

    return getHistory(query);
}

QVector<TimeLogSyncData> TimeLogHistoryWorker::getSyncAffected(const QUuid &uuid)
{
    if (!m_syncAffectedQuery) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString("WITH result AS ( "
                            "    SELECT uuid, start, category, comment, mtime FROM timelog "
                            "    WHERE uuid=:uuid "
                            "UNION ALL "
                            "    SELECT uuid, NULL, NULL, NULL, mtime FROM removed "
                            "    WHERE uuid=:uuid "
                            ") "
                            "SELECT * FROM result ORDER BY mtime DESC LIMIT 1");
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(query->lastError().text());
            delete query;
            return QVector<TimeLogSyncData>();
        }

        m_syncAffectedQuery = query;
    }

    m_syncAffectedQuery->bindValue(":uuid", uuid.toRfc4122());

    return getSyncData(*m_syncAffectedQuery);
}

QMap<QDateTime, QByteArray> TimeLogHistoryWorker::getDataHashes(const QDateTime &maxDate) const
{
    QMap<QDateTime, QByteArray> result;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString = QString("SELECT start, hash FROM hashes %1 ORDER BY start ASC")
                                  .arg(maxDate.isValid() ? "WHERE start <= ?" : "");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return result;
    }
    if (maxDate.isValid()) {
        query.addBindValue(maxDate.toTime_t());
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return result;
    }

    while (query.next()) {
        result.insert(QDateTime::fromTime_t(query.value(0).toUInt(), Qt::UTC),
                      query.value(1).toByteArray());
    }

    query.finish();

    return result;
}

void TimeLogHistoryWorker::notifyInsertUpdates(const TimeLogEntry &data)
{
    if (!m_notifyInsertQuery) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString = QString("SELECT * FROM ( "
                                      "    %1 WHERE start <= :newStart ORDER BY start DESC LIMIT 2 "
                                      ") "
                                      "UNION "
                                      "SELECT * FROM ( "
                                      "    %1 WHERE start > :newStart ORDER BY start ASC LIMIT 1 "
                                      ") ORDER BY start ASC").arg(selectFields);
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(query->lastError().text());
            delete query;
            return;
        }

        m_notifyInsertQuery = query;
    }

    m_notifyInsertQuery->bindValue(":newStart", data.startTime.toTime_t());

    notifyUpdates(*m_notifyInsertQuery);
}

void TimeLogHistoryWorker::notifyInsertUpdates(const QVector<TimeLogEntry> &data)
{
    foreach (const TimeLogEntry &entry, data) {
        notifyInsertUpdates(entry);    // TODO: optimize
    }
}

void TimeLogHistoryWorker::notifyRemoveUpdates(const TimeLogEntry &data)
{
    if (!m_notifyRemoveQuery) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString = QString("SELECT * FROM ( "
                                      "    %1 WHERE start < :oldStart ORDER BY start DESC LIMIT 1 "
                                      ") "
                                      "UNION "
                                      "SELECT * FROM ( "
                                      "    %1 WHERE start > :oldStart ORDER BY start ASC LIMIT 1 "
                                      ") ORDER BY start ASC").arg(selectFields);
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(query->lastError().text());
            delete query;
            return;
        }

        m_notifyRemoveQuery = query;
    }

    m_notifyRemoveQuery->bindValue(":oldStart", data.startTime.toTime_t());

    notifyUpdates(*m_notifyRemoveQuery);
}

void TimeLogHistoryWorker::notifyEditUpdates(const TimeLogEntry &data, TimeLogHistory::Fields fields, QDateTime oldStart)
{
    QSqlQuery **requestQuery = (fields & TimeLogHistory::StartTime ? &m_notifyEditStartQuery
                                                                   : &m_notifyEditQuery);
    if (!(*requestQuery)) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString;
        if (fields & TimeLogHistory::StartTime) {
            queryString = QString("SELECT * FROM ( "
                                  "    %1 WHERE start <= :newStart ORDER BY start DESC LIMIT 2 "
                                  ") "
                                  "UNION "
                                  "SELECT * FROM ( "
                                  "    %1 WHERE start > :newStart ORDER BY start ASC LIMIT 1 "
                                  ") "
                                  "UNION "
                                  "SELECT * FROM ( "
                                  "    %1 WHERE start < :oldStart ORDER BY start DESC LIMIT 1 "
                                  ") "
                                  "UNION "
                                  "SELECT * FROM ( "
                                  "    %1 WHERE start > :oldStart ORDER BY start ASC LIMIT 1 "
                                  ") ORDER BY start ASC").arg(selectFields);
        } else {
            queryString = QString("%1 WHERE start=:start").arg(selectFields);   // TODO: lookup by uuid
        }
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(query->lastError().text());
            delete query;
            return;
        }

        *requestQuery = query;
    }

    if (fields & TimeLogHistory::StartTime) {
        (*requestQuery)->bindValue(":newStart", data.startTime.toTime_t());
        (*requestQuery)->bindValue(":oldStart", oldStart.toTime_t());
        fields |= TimeLogHistory::DurationTime | TimeLogHistory::PrecedingStart;
    } else {
        (*requestQuery)->bindValue(":start", data.startTime.toTime_t());
    }

    notifyUpdates(**requestQuery, fields);
}

void TimeLogHistoryWorker::notifyUpdates(QSqlQuery &query, TimeLogHistory::Fields fields) const
{
    QVector<TimeLogEntry> updatedData = getHistory(query);
    QVector<TimeLogHistory::Fields> updatedFields;

    if (!updatedData.isEmpty()) {
        qCDebug(HISTORY_WORKER_CATEGORY) << "Updated items count:" << updatedData.size();
        updatedFields.insert(0, updatedData.size(), fields);
        emit dataUpdated(updatedData, updatedFields);
    }
}

bool TimeLogHistoryWorker::updateSize()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString = QString("SELECT count(*) FROM timelog");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return false;
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery();
        emit error(query.lastError().text());
        return false;
    }

    query.next();
    setSize(query.value(0).toULongLong());
    query.finish();

    return true;
}

bool TimeLogHistoryWorker::updateCategories(const QDateTime &begin, const QDateTime &end)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("SELECT DISTINCT category FROM timelog"
                        " WHERE start BETWEEN ? AND ?");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return false;
    }
    query.addBindValue(begin.toTime_t());
    query.addBindValue(end.toTime_t());

    QSet<QString> result;

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return false;
    }

    while (query.next()) {
        result.insert(query.value(0).toString());
    }

    query.finish();

    m_categorySet.swap(result);
    m_categoryTree = parseCategories(m_categorySet);

    emit categoriesChanged(m_categoryTree);

    return true;
}

QSharedPointer<TimeLogCategory> TimeLogHistoryWorker::parseCategories(const QSet<QString> &categories) const
{
    QStringList categoriesList = categories.toList();
    std::sort(categoriesList.begin(), categoriesList.end());

    TimeLogCategory *rootCategory = new TimeLogCategory("RootCategory");

    foreach (const QString &category, categoriesList) {
        QStringList categoryFields = category.split(m_categorySplitRegexp, QString::SkipEmptyParts);
        TimeLogCategory *parentCategory = rootCategory;
        foreach (const QString &categoryField, categoryFields) {
            TimeLogCategory *categoryObject = parentCategory->children().value(categoryField);
            if (!categoryObject) {
                categoryObject = new TimeLogCategory(categoryField, parentCategory);
            }
            parentCategory = categoryObject;
        }
    }

    return QSharedPointer<TimeLogCategory>(rootCategory);
}

QByteArray TimeLogHistoryWorker::calcHash(const QDateTime &begin, const QDateTime &end) const
{
    QByteArray result;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("WITH result AS ( "
                        "    SELECT mtime, uuid FROM timelog "
                        "    WHERE (mtime BETWEEN :mBegin AND :mEnd) "
                        "UNION ALL "
                        "    SELECT mtime, uuid FROM removed "
                        "    WHERE (mtime BETWEEN :mBegin AND :mEnd) "
                        ") "
                        "SELECT * FROM result ORDER BY mtime ASC, uuid ASC");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return result;
    }
    query.bindValue(":mBegin", begin.toMSecsSinceEpoch());
    query.bindValue(":mEnd", end.toMSecsSinceEpoch());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return result;
    }

    QByteArray hashData;
    QDataStream dataStream(&hashData, QIODevice::WriteOnly);

    while (query.next()) {
        dataStream << query.value(0).toLongLong();
        dataStream << query.value(1).toByteArray();
    }

    query.finish();

    if (!hashData.isEmpty()) {
        result = QCryptographicHash::hash(hashData, QCryptographicHash::Md5);
    }

    return result;
}

void TimeLogHistoryWorker::pushUndo(const TimeLogHistoryWorker::Undo undo)
{
    m_undoStack.push(undo);

    if (m_undoStack.size() > maxUndoSize) {
        m_undoStack.takeFirst();
    } else {
        emit undoCountChanged(m_undoStack.size());
    }
}
