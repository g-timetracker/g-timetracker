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

#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QDataStream>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>

#include <QLoggingCategory>

#include "TimeLogHistoryWorker.h"
#include "TimeLogCategoryTreeNode.h"
#include "TimeLogDefaultCategories.h"

Q_LOGGING_CATEGORY(HISTORY_WORKER_CATEGORY, "TimeLogHistoryWorker", QtInfoMsg)

const qint32 dbSchemaVersion = 1;

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
    if (m_isInitialized) {
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
}

bool TimeLogHistoryWorker::init(const QString &dataPath, const QString &filePath, bool isReadonly, bool isPopulateCategories)
{
    Q_ASSERT(!m_isInitialized);

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
    if (isReadonly) {
        db.setConnectOptions("QSQLITE_OPEN_READONLY");
    }

    if (!db.open()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to open db:" << db.lastError().text();
        return false;
    }

    qlonglong schemaVersion = getSchemaVersion();
    switch (schemaVersion) {
    case -1:
        return false;
    case 0: // clean db
        if (!isReadonly && !setSchemaVersion(dbSchemaVersion)) {
            return false;
        }
        // fall through
    case dbSchemaVersion:
        break;
    default:
        qCCritical(HISTORY_WORKER_CATEGORY) << "Unsupported DB schema version:" << schemaVersion;
        return false;
    }

    if (!setupTable()) {
        return false;
    }

    if (!setupTriggers()) {
        return false;
    }

    if (!fetchCategories()) {
        return false;
    }

    if (isPopulateCategories && checkIsDBEmpty()) {
        if (!populateCategories()) {
            return false;
        }
    }

    m_isInitialized = true;

    return true;
}

void TimeLogHistoryWorker::deinit()
{
    Q_ASSERT(m_isInitialized);

    delete m_insertQuery;
    m_insertQuery = nullptr;
    delete m_removeQuery;
    m_removeQuery = nullptr;
    delete m_notifyInsertQuery;
    m_notifyInsertQuery = nullptr;
    delete m_notifyRemoveQuery;
    m_notifyRemoveQuery = nullptr;
    delete m_notifyEditQuery;
    m_notifyEditQuery = nullptr;
    delete m_notifyEditStartQuery;
    m_notifyEditStartQuery = nullptr;
    delete m_syncAffectedQuery;
    m_syncAffectedQuery = nullptr;
    delete m_entryQuery;
    m_entryQuery = nullptr;

    QSqlDatabase::database(m_connectionName).close();
    QSqlDatabase::removeDatabase(m_connectionName);
    m_connectionName.clear();

    setSize(0);

    m_categories.clear();
    m_categoryRecordsCount.clear();
    updateCategories();

    if (!m_undoStack.isEmpty()) {
        m_undoStack.clear();
        emit undoCountChanged(0);
    }

    m_isInitialized = false;
}

qlonglong TimeLogHistoryWorker::size() const
{
    return m_size;
}

QSharedPointer<TimeLogCategoryTreeNode> TimeLogHistoryWorker::categories() const
{
    return m_categoryTree;
}

void TimeLogHistoryWorker::insert(const TimeLogEntry &data)
{
    Q_ASSERT(m_isInitialized);

    Undo undo;
    undo.type = Undo::InsertEntry;
    undo.entryData.append(data);
    pushUndo(undo);

    insertEntry(data);
}

void TimeLogHistoryWorker::import(const QVector<TimeLogEntry> &data)
{
    Q_ASSERT(m_isInitialized);

    if (insertEntryData(data) && fetchCategories()) {
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
    undo.type = Undo::RemoveEntry;
    undo.entryData.append(entry);
    pushUndo(undo);

    removeEntry(data);
}

void TimeLogHistoryWorker::edit(const TimeLogEntry &data, TimeLogHistory::Fields fields)
{
    Q_ASSERT(m_isInitialized);

    TimeLogEntry entry = getEntry(data.uuid);
    Undo undo;
    undo.type = Undo::EditEntry;
    undo.entryData.append(entry);
    undo.entryFields.append(fields);
    pushUndo(undo);

    editEntry(data, fields);
}

void TimeLogHistoryWorker::addCategory(const TimeLogCategory &category)
{
    Q_ASSERT(m_isInitialized);

    QString categoryName(fixCategoryName(category.name));

    if (categoryName.isEmpty()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << QString("Empty category name");
        emit error("Category name can not be empty");
        return;
    } else if (m_categories.contains(categoryName) && m_categories.value(categoryName).isValid()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << QString("Category '%1' already exists").arg(categoryName);
        emit error(QString("Category '%1' already exists").arg(categoryName));
        return;
    }

    TimeLogCategory newCategory(category);
    newCategory.name = categoryName;

    Undo undo;
    undo.type = Undo::AddCategory;
    undo.categoryData = newCategory;
    pushUndo(undo);

    addCategoryData(newCategory);
}

void TimeLogHistoryWorker::removeCategory(const QString &name)
{
    Q_ASSERT(m_isInitialized);

    if (name.isEmpty()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << QString("Empty category name");
        emit error(tr("Category name can not be empty"));
        return;
    } else if (!m_categories.contains(name)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << QString("No such category: %1").arg(name);
        emit error(tr("No such category: %1").arg(name));
        return;
    }

    Undo undo;
    undo.type = Undo::RemoveCategory;
    undo.categoryData = m_categories.value(name);
    if (undo.categoryData.uuid.isNull()) {  // Entry-only category
        undo.categoryData.uuid = QUuid::createUuid();
    }
    pushUndo(undo);

    removeCategoryData(undo.categoryData);
}

void TimeLogHistoryWorker::editCategory(const QString &oldName, const TimeLogCategory &category)
{
    Q_ASSERT(m_isInitialized);

    QString categoryName(fixCategoryName(category.name));

    if (categoryName.isEmpty()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << QString("Empty category name");
        emit error(tr("Category name can not be empty"));
        return;
    } else if (!m_categories.contains(oldName)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << QString("No such category: %1").arg(oldName);
        emit error(tr("No such category: %1").arg(oldName));
        return;
    }

    TimeLogCategory newCategory(category);
    newCategory.name = categoryName;

    TimeLogCategory oldCategory = m_categories.value(oldName);
    QVector<TimeLogEntry> entries = getEntries(oldName);
    Undo undo;
    if (oldName != categoryName && m_categories.contains(categoryName)) {
        undo.type = Undo::MergeCategories;
        if (oldCategory.uuid.isNull()) {    // Entry-only category
            oldCategory.uuid = QUuid::createUuid();
        }
    } else {
        undo.type = Undo::EditCategory;
    }
    QVector<TimeLogEntry> editedEntries;
    if (oldName != categoryName) {
        editedEntries.reserve(entries.size());
        for (const TimeLogEntry &entry: entries) {
            TimeLogEntry editedEntry(entry);
            editedEntry.category = categoryName;
            editedEntries.append(editedEntry);
        }
        undo.entryData.swap(entries);
        undo.entryFields.insert(0, undo.entryData.size(), TimeLogHistory::Category);
    }
    undo.categoryData = oldCategory;
    if (undo.type == Undo::EditCategory && undo.categoryData.uuid.isNull()) {   // Entry-only category
        undo.categoryData.uuid = newCategory.uuid;
    }
    undo.categoryNewName = categoryName;
    pushUndo(undo);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);

    if (!startTransaction(db)) {
        goto fail;
    }

    if (!editedEntries.isEmpty()) {
        if (!editEntriesCategory(oldName, categoryName)) {
            goto rollback;
        }
        for (const TimeLogEntry &entry: editedEntries) {
            notifyEditUpdates(entry, TimeLogHistory::Category);
        }
    }

    if (undo.type == Undo::EditCategory) {
        if (!oldCategory.isValid()) {  // Entry-only category, need to create db record
            if (!addCategoryData(newCategory)) {
                goto rollback;
            }
        } else {
            if (!editCategoryData(oldName, newCategory)) {
                goto rollback;
            }
        }
    } else {    // merge
        if (!removeCategoryData(oldCategory)) {
            goto rollback;
        }
    }

    if (!commitTransaction(db)) {
        goto fail;
    }

    return;

rollback:
    rollbackTransaction(db);

fail:
    processFail();
}

void TimeLogHistoryWorker::sync(const QVector<TimeLogSyncDataEntry> &updatedData,
                                const QVector<TimeLogSyncDataEntry> &removedData,
                                const QVector<TimeLogSyncDataCategory> &categoryData)
{
    Q_ASSERT(m_isInitialized);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!startTransaction(db)) {
        return;
    }

    QDateTime maxEntrySyncDate, maxCategorySyncDate;

    if (!syncCategories(categoryData, maxCategorySyncDate)
        || !syncEntries(updatedData, removedData, maxEntrySyncDate)) {
        rollbackTransaction(db);
        return;
    } else if (!commitTransaction(db)) {
        return;
    } else {
        emit dataSynced(qMax(maxEntrySyncDate, maxCategorySyncDate));
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
    case Undo::InsertEntry:
        removeEntry(undo.entryData.constFirst());
        break;
    case Undo::RemoveEntry:
        insertEntry(undo.entryData.constFirst());
        break;
    case Undo::EditEntry:
        editEntry(undo.entryData.constFirst(), undo.entryFields.constFirst());
        break;
    case Undo::AddCategory:
        removeCategoryData(undo.categoryData);
        break;
    case Undo::RemoveCategory:
        addCategoryData(undo.categoryData);
        break;
    case Undo::EditCategory: {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);

        if (!startTransaction(db)) {
            break;
        }

        if (!editCategoryData(undo.categoryNewName, undo.categoryData)) {
            rollbackTransaction(db);
            break;
        }

        if (!undo.entryData.isEmpty() && !editEntries(undo.entryData, undo.entryFields)) {
            rollbackTransaction(db);
            break;
        }

        if (!commitTransaction(db)) {
            break;
        }

        break;
    }
    case Undo::MergeCategories: {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);

        if (!startTransaction(db)) {
            break;
        }

        if (!addCategoryData(undo.categoryData)) {
            rollbackTransaction(db);
            break;
        }

        if (!undo.entryData.isEmpty() && !editEntries(undo.entryData, undo.entryFields)) {
            rollbackTransaction(db);
            break;
        }

        if (!commitTransaction(db)) {
            break;
        }

        break;
    }
    }

    emit undoCountChanged(m_undoStack.size());
}

void TimeLogHistoryWorker::getHistoryBetween(qlonglong id, const QDateTime &begin, const QDateTime &end,
                                             const QString &category, bool withSubcategories) const
{
    Q_ASSERT(m_isInitialized);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString = QString("%1 WHERE (start BETWEEN ? AND ?) %2 ORDER BY start ASC")
                                  .arg(selectFields)
                                  .arg(category.isEmpty() ? ""
                                                          : QString("AND category %1")
                                                            .arg(withSubcategories ? "LIKE ? || '%'"
                                                                                   : "=?"));
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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

void TimeLogHistoryWorker::getStoredCategories() const
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("SELECT uuid, category, data FROM categories ORDER BY category ASC ");
    if (!prepareAndExecQuery(query, queryString)) {
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return;
    }

    QVector<TimeLogCategory> result;

    while (query.next()) {
        TimeLogCategory category;

        category.uuid = QUuid::fromRfc4122(query.value(0).toByteArray());
        category.name = query.value(1).toString();

        if (!query.value(2).isNull()) {
            QByteArray jsonString = query.value(2).toByteArray();
            QJsonParseError parseError;
            QJsonDocument document = QJsonDocument::fromJson(jsonString, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to parse category data JSON:"
                                                    << jsonString << "error at offset"
                                                    << parseError.offset
                                                    << parseError.errorString() << parseError.error;
                emit error(tr("Fail to parse category data: %1").arg(parseError.errorString()));
                return;
            }
            category.data = document.object().toVariantMap();
        }

        result.append(category);
    }

    query.finish();

    emit storedCategoriesAvailable(result);
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
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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

    emit syncDataAvailable(getSyncEntryData(mBegin, mEnd), getSyncCategoryData(mBegin, mEnd), mEnd);
}

void TimeLogHistoryWorker::getSyncExists(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    Q_ASSERT(m_isInitialized);

    emit syncExistsAvailable(getSyncDataExists(mBegin, mEnd), mBegin, mEnd);
}

void TimeLogHistoryWorker::getSyncAmount(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    Q_ASSERT(m_isInitialized);

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("SELECT count(*), max(mtime) FROM ( "
                        "    SELECT mtime FROM timelog "
                        "    WHERE mtime BETWEEN :mBegin AND :mEnd "
                        "UNION "
                        "    SELECT mtime FROM timelog_removed "
                        "    WHERE mtime BETWEEN :mBegin AND :mEnd "
                        "UNION "
                        "    SELECT mtime FROM categories "
                        "    WHERE mtime BETWEEN :mBegin AND :mEnd "
                        "UNION "
                        "    SELECT mtime FROM categories_removed "
                        "    WHERE mtime BETWEEN :mBegin AND :mEnd "
                        ")");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return;
    }
    query.bindValue(":mBegin", mBegin.toMSecsSinceEpoch());
    query.bindValue(":mEnd", mEnd.toMSecsSinceEpoch());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return;
    }

    query.next();
    qlonglong result = query.value(0).toLongLong();
    QDateTime maxMTime;
    if (!query.isNull(1)) {
        maxMTime = QDateTime::fromMSecsSinceEpoch(query.value(1).toLongLong(), Qt::UTC);
    }
    query.finish();

    emit syncAmountAvailable(result, maxMTime, mBegin, mEnd);
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

bool TimeLogHistoryWorker::prepareAndExecQuery(QSqlQuery &query, const QString &queryString) const
{
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

qlonglong TimeLogHistoryWorker::getSchemaVersion() const
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("PRAGMA user_version;");
    if (!prepareAndExecQuery(query, queryString)) {
        return -1;
    }

    if (!query.next()) {
        return 0;
    }

    return query.value(0).toLongLong();
}

bool TimeLogHistoryWorker::setSchemaVersion(qint32 schemaVersion)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString(QString("PRAGMA user_version = %1;").arg(QString().setNum(schemaVersion)));

    return prepareAndExecQuery(query, queryString);
}

bool TimeLogHistoryWorker::setupTable()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString;

    /* timelog */
    queryString = "CREATE TABLE IF NOT EXISTS timelog"
                  " (uuid BLOB UNIQUE NOT NULL, start INTEGER PRIMARY KEY, category TEXT NOT NULL,"
                  " comment TEXT, duration INTEGER, mtime INTEGER);";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE INDEX IF NOT EXISTS timelog_category_index ON timelog (category);";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE INDEX IF NOT EXISTS timelog_mtime_index ON timelog (mtime);";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    /* timelog removed */
    queryString = "CREATE TABLE IF NOT EXISTS timelog_removed"
                  "(uuid BLOB PRIMARY KEY, mtime INTEGER) WITHOUT ROWID;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE INDEX IF NOT EXISTS timelog_removed_mtime_index ON timelog_removed (mtime);";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    /* categories */
    queryString = "CREATE TABLE IF NOT EXISTS categories"
                  " (uuid BLOB PRIMARY KEY, category TEXT UNIQUE NOT NULL, data BLOB, mtime INTEGER) WITHOUT ROWID;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE INDEX IF NOT EXISTS categories_mtime_index ON categories (mtime);";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    /* categories removed */
    queryString = "CREATE TABLE IF NOT EXISTS categories_removed"
                  " (uuid BLOB PRIMARY KEY, mtime INTEGER) WITHOUT ROWID;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE INDEX IF NOT EXISTS categories_removed_mtime_index ON categories_removed (mtime);";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    /* hashes */
    queryString = "CREATE TABLE IF NOT EXISTS hashes (start INTEGER PRIMARY KEY, hash BLOB);";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    return true;
}

bool TimeLogHistoryWorker::setupTriggers()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString;

    /* timelog */
    queryString = "CREATE TRIGGER IF NOT EXISTS check_insert_timelog BEFORE INSERT ON timelog "
                  "BEGIN "
                  "    SELECT mtime, "
                  "        CASE WHEN NEW.mtime < mtime "
                  "            THEN RAISE(IGNORE) "
                  "        END "
                  "    FROM timelog_removed WHERE uuid=NEW.uuid; "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
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
                  "    DELETE FROM timelog_removed WHERE uuid=NEW.uuid; "
                  "    INSERT OR REPLACE INTO hashes (start, hash) VALUES(strftime('%s', NEW.mtime/1000, 'unixepoch', 'start of month'), NULL); "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
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
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS check_update_timelog BEFORE UPDATE ON timelog "
                  "BEGIN "
                  "    SELECT "
                  "        CASE WHEN NEW.mtime < OLD.mtime "
                  "            THEN RAISE(IGNORE) "
                  "        END; "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    // Update hash on change of the any field, except duration
    queryString = "CREATE TRIGGER IF NOT EXISTS update_timelog AFTER UPDATE OF start, category, comment, mtime, uuid ON timelog "
                  "BEGIN "
                  "    INSERT OR REPLACE INTO hashes (start, hash) VALUES(strftime('%s', NEW.mtime/1000, 'unixepoch', 'start of month'), NULL); "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS update_timelog_start AFTER UPDATE OF start ON timelog "
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
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    /* timelog removed */
    queryString = "CREATE TRIGGER IF NOT EXISTS check_insert_timelog_removed BEFORE INSERT ON timelog_removed "
                  "BEGIN "
                  "    SELECT mtime, "
                  "        CASE WHEN NEW.mtime < mtime "
                  "            THEN RAISE(IGNORE) "
                  "        END "
                  "    FROM timelog_removed WHERE uuid=NEW.uuid; "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS insert_timelog_removed AFTER INSERT ON timelog_removed "
                  "BEGIN "
                  "    DELETE FROM timelog WHERE uuid=NEW.uuid; "
                  "    INSERT OR REPLACE INTO hashes (start, hash) VALUES(strftime('%s', NEW.mtime/1000, 'unixepoch', 'start of month'), NULL); "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    /* categories */
    queryString = "CREATE TRIGGER IF NOT EXISTS check_insert_categories BEFORE INSERT ON categories "
                  "BEGIN "
                  "    SELECT mtime, "
                  "        CASE WHEN NEW.mtime < mtime "
                  "            THEN RAISE(IGNORE) "
                  "        END "
                  "    FROM categories_removed WHERE uuid=NEW.uuid; "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS insert_categories AFTER INSERT ON categories "
                  "BEGIN "
                  "    DELETE FROM categories_removed WHERE uuid=NEW.uuid; "
                  "    INSERT OR REPLACE INTO hashes (start, hash) VALUES(strftime('%s', NEW.mtime/1000, 'unixepoch', 'start of month'), NULL); "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS check_update_categories BEFORE UPDATE ON categories "
                  "BEGIN "
                  "    SELECT "
                  "        CASE WHEN NEW.mtime < OLD.mtime "
                  "            THEN RAISE(IGNORE) "
                  "        END; "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS update_categories AFTER UPDATE ON categories "
                  "BEGIN "
                  "    INSERT OR REPLACE INTO hashes (start, hash) VALUES(strftime('%s', NEW.mtime/1000, 'unixepoch', 'start of month'), NULL); "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    /* categories removed */
    queryString = "CREATE TRIGGER IF NOT EXISTS check_insert_categories_removed BEFORE INSERT ON categories_removed "
                  "BEGIN "
                  "    SELECT mtime, "
                  "        CASE WHEN NEW.mtime < mtime "
                  "            THEN RAISE(IGNORE) "
                  "        END "
                  "    FROM categories_removed WHERE uuid=NEW.uuid; "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
        return false;
    }

    queryString = "CREATE TRIGGER IF NOT EXISTS insert_categories_removed AFTER INSERT ON categories_removed "
                  "BEGIN "
                  "    DELETE FROM categories WHERE uuid=NEW.uuid; "
                  "    INSERT OR REPLACE INTO hashes (start, hash) VALUES(strftime('%s', NEW.mtime/1000, 'unixepoch', 'start of month'), NULL); "
                  "END;";
    if (!prepareAndExecQuery(query, queryString)) {
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

void TimeLogHistoryWorker::decrementCategoryCount(const QString &name)
{
    if (!m_categoryRecordsCount.contains(name)) {
        fetchCategories();
        return;
    } else if (--(m_categoryRecordsCount[name]) == 0) {
        updateCategories();
    }
}

void TimeLogHistoryWorker::incrementCategoryCount(const QString &name)
{
    if (!m_categoryRecordsCount.contains(name)) {
        m_categories.insert(name, TimeLogCategory(QUuid(), TimeLogCategoryData(name)));
        m_categoryRecordsCount.insert(name, 1);
    } else if ((m_categoryRecordsCount[name])++ > 0) {
        return;
    }

    updateCategories();
}

void TimeLogHistoryWorker::processFail()
{
    m_undoStack.clear();
    emit undoCountChanged(0);

    emit dataOutdated();
}

void TimeLogHistoryWorker::insertEntry(const TimeLogEntry &data)
{
    if (insertEntryData(data)) {
        emit dataInserted(data);
        notifyInsertUpdates(data);

        incrementCategoryCount(data.category);
    } else {
        processFail();
    }
}

void TimeLogHistoryWorker::removeEntry(const TimeLogEntry &data)
{
    if (removeEntryData(data)) {
        emit dataRemoved(data);
        notifyRemoveUpdates(data);

        decrementCategoryCount(data.category);
    } else {
        processFail();
    }
}

bool TimeLogHistoryWorker::editEntry(const TimeLogEntry &data, TimeLogHistory::Fields fields)
{
    QDateTime oldStart;
    QString oldCategory;
    if (fields == TimeLogHistory::NoFields) {
        qCWarning(HISTORY_WORKER_CATEGORY) << "No fields specified";
        return false;
    } else if (fields & (TimeLogHistory::StartTime | TimeLogHistory::Category)) {
        TimeLogEntry oldData = getEntry(data.uuid);
        if (!oldData.isValid()) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Item to update not found:\n"
                                                << data.startTime << data.category << data.uuid;
            processFail();
            return false;
        }
        if (fields & TimeLogHistory::StartTime) {
            oldStart = oldData.startTime;
        }
        if (fields & TimeLogHistory::Category) {
            oldCategory = oldData.category;
        }
    }

    if (editEntryData(data, fields)) {
        notifyEditUpdates(data, fields, oldStart);

        if (fields & TimeLogHistory::Category) {
            decrementCategoryCount(oldCategory);
            incrementCategoryCount(data.category);
        }
    }  else {
        processFail();
        return false;
    }

    return true;
}

bool TimeLogHistoryWorker::editEntries(const QVector<TimeLogEntry> &data, const QVector<TimeLogHistory::Fields> &fields)
{
    for (int i = 0; i < data.size(); i++) {
        if (!editEntry(data.at(i), fields.at(i))) {
            return false;
        }
    }

    return true;
}

bool TimeLogHistoryWorker::syncEntries(const QVector<TimeLogSyncDataEntry> &updatedData,
                                       const QVector<TimeLogSyncDataEntry> &removedData,
                                       QDateTime &maxSyncDate)
{
    QVector<TimeLogSyncDataEntry> removedNew;
    QVector<TimeLogSyncDataEntry> removedOld;
    QVector<TimeLogSyncDataEntry> insertedNew;
    QVector<TimeLogSyncDataEntry> insertedOld;
    QVector<TimeLogSyncDataEntry> updatedNew;
    QVector<TimeLogSyncDataEntry> updatedOld;
    QVector<TimeLogHistory::Fields> updateFields;

    for (const TimeLogSyncDataEntry &item: removedData) {
        QVector<TimeLogSyncDataEntry> affected = getSyncEntriesAffected(item.entry.uuid);
        if (!affected.isEmpty() && affected.constFirst().sync.mTime >= item.sync.mTime) {
            continue;
        }

        removedNew.append(item);
        removedOld.append(affected.isEmpty() ? TimeLogSyncDataEntry() : affected.constFirst());
    }

    for (const TimeLogSyncDataEntry &item: updatedData) {
        QVector<TimeLogSyncDataEntry> affected = getSyncEntriesAffected(item.entry.uuid);
        if (!affected.isEmpty() && affected.constFirst().sync.mTime >= item.sync.mTime) {
            continue;
        }

        if (affected.isEmpty() || !affected.constFirst().entry.isValid()) {
            insertedNew.append(item);
            insertedOld.append(affected.isEmpty() ? TimeLogSyncDataEntry() : affected.constFirst());
        } else {
            const TimeLogSyncDataEntry &oldItem(affected.constFirst());
            TimeLogHistory::Fields fields(TimeLogHistory::NoFields);
            if (item.entry.startTime != oldItem.entry.startTime) {
                fields |= TimeLogHistory::StartTime;
            }
            if (item.entry.category != oldItem.entry.category) {
                fields |= TimeLogHistory::Category;
            }
            if (item.entry.comment != oldItem.entry.comment) {
                fields |= TimeLogHistory::Comment;
            }
            updatedNew.append(item);
            updatedOld.append(oldItem);
            updateFields.append(fields);
        }
    }

    emit syncEntryStatsAvailable(removedOld, removedNew, insertedOld, insertedNew, updatedOld, updatedNew);

    QVector<TimeLogSyncDataEntry> removedMerged(removedOld);
    for (int i = 0; i < removedMerged.size(); i++) {
        removedMerged[i].entry.uuid = removedNew.at(i).entry.uuid;
        removedMerged[i].sync.mTime = removedNew.at(i).sync.mTime;
    }

    if (!syncEntryData(removedMerged, insertedNew, updatedNew, updateFields)) {
        return false;
    }

    for (const TimeLogSyncDataEntry &item: removedMerged) {
        if (item.entry.isValid()) {
            emit dataRemoved(item.entry);
        }
    }
    for (const TimeLogSyncDataEntry &item: removedMerged) {
        if (item.entry.isValid()) {
            notifyRemoveUpdates(item.entry);
        }
    }
    for (const TimeLogSyncDataEntry &item: insertedNew) {
        emit dataInserted(item.entry);
    }
    for (const TimeLogSyncDataEntry &item: insertedNew) {
        notifyInsertUpdates(item.entry);
    }
    for (int i = 0; i < updatedNew.size(); i++) {
        notifyEditUpdates(updatedNew.at(i).entry, updateFields.at(i), updatedOld.at(i).entry.startTime);
    }

    for (const TimeLogSyncDataEntry &item: removedOld) {
        if (item.entry.isValid()) {
            decrementCategoryCount(item.entry.category);
        }
    }
    for (const TimeLogSyncDataEntry &item: insertedNew) {
        incrementCategoryCount(item.entry.category);
    }
    for (int i = 0; i < updatedNew.size(); i++) {
        decrementCategoryCount(updatedOld.at(i).entry.category);
        incrementCategoryCount(updatedNew.at(i).entry.category);
    }

    if (!removedNew.isEmpty()) {
        maxSyncDate = removedNew.constLast().sync.mTime;
    }
    if (!insertedNew.isEmpty() && insertedNew.constLast().sync.mTime > maxSyncDate) {
        maxSyncDate = insertedNew.constLast().sync.mTime;
    }
    if (!updatedNew.isEmpty() && updatedNew.constLast().sync.mTime > maxSyncDate) {
        maxSyncDate = updatedNew.constLast().sync.mTime;
    }

    return true;
}

bool TimeLogHistoryWorker::syncCategories(const QVector<TimeLogSyncDataCategory> &categoryData,
                                          QDateTime &maxSyncDate)
{
    QVector<TimeLogSyncDataCategory> removedNew;
    QVector<TimeLogSyncDataCategory> removedOld;
    QVector<TimeLogSyncDataCategory> addedNew;
    QVector<TimeLogSyncDataCategory> addedOld;
    QVector<TimeLogSyncDataCategory> updatedNew;
    QVector<TimeLogSyncDataCategory> updatedOld;

    for (const TimeLogSyncDataCategory &item: categoryData) {
        QVector<TimeLogSyncDataCategory> affected = getSyncCategoriesAffected(item.category.uuid);
        if (!affected.isEmpty() && affected.constFirst().sync.mTime >= item.sync.mTime) {
            continue;
        }

        if (!item.category.isValid()) {
            removedNew.append(item);
            removedOld.append(affected.isEmpty() ? TimeLogSyncDataCategory() : affected.constFirst());
        } else {
            if (affected.isEmpty() || !affected.constFirst().category.isValid()) {
                addedNew.append(item);
                addedOld.append(affected.isEmpty() ? TimeLogSyncDataCategory() : affected.constFirst());
            } else {
                updatedNew.append(item);
                updatedOld.append(affected.constFirst());
            }
        }
    }

    emit syncCategoryStatsAvailable(removedOld, removedNew, addedOld, addedNew, updatedOld, updatedNew);

    QVector<TimeLogSyncDataCategory> removedMerged(removedOld);
    for (int i = 0; i < removedMerged.size(); i++) {
        removedMerged[i].category.uuid = removedNew.at(i).category.uuid;
        removedMerged[i].sync.mTime = removedNew.at(i).sync.mTime;
    }

    if (!syncCategoryData(removedMerged, addedNew, updatedNew, updatedOld)) {
        return false;
    }

    if (!removedNew.isEmpty()) {
        maxSyncDate = removedNew.constLast().sync.mTime;
    }
    if (!addedNew.isEmpty() && addedNew.constLast().sync.mTime > maxSyncDate) {
        maxSyncDate = addedNew.constLast().sync.mTime;
    }
    if (!updatedNew.isEmpty() && updatedNew.constLast().sync.mTime > maxSyncDate) {
        maxSyncDate = updatedNew.constLast().sync.mTime;
    }

    return true;
}

bool TimeLogHistoryWorker::insertEntryData(const QVector<TimeLogEntry> &data)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!startTransaction(db)) {
        return false;
    }

    for (const TimeLogEntry &entry: data) {
        if (!insertEntryData(entry)) {
            rollbackTransaction(db);
            return false;
        }
    }

    if (!commitTransaction(db)) {
        return false;
    }

    return true;
}

bool TimeLogHistoryWorker::insertEntryData(const TimeLogSyncDataEntry &data)
{
    Q_ASSERT(data.entry.isValid());

    if (!m_insertQuery) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString = QString("INSERT INTO timelog (uuid, start, category, comment, mtime)"
                                      " VALUES (:uuid, :start, :category, :comment, :mtime);");
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(tr("DB error: %1").arg(query->lastError().text()));
            delete query;
            return false;
        }

        m_insertQuery = query;
    }

    m_insertQuery->bindValue(":uuid", data.entry.uuid.toRfc4122());
    m_insertQuery->bindValue(":start", data.entry.startTime.toTime_t());
    m_insertQuery->bindValue(":category", data.entry.category);
    m_insertQuery->bindValue(":comment", data.entry.comment);
    m_insertQuery->bindValue(":mtime", data.sync.mTime.isValid() ? data.sync.mTime.toMSecsSinceEpoch()
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

    return true;
}

bool TimeLogHistoryWorker::removeEntryData(const TimeLogSyncDataEntry &data)
{
    Q_ASSERT(!data.entry.uuid.isNull());

    if (!m_removeQuery) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString("INSERT OR REPLACE INTO timelog_removed (uuid, mtime) VALUES(:uuid,:mtime);");
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(tr("DB error: %1").arg(query->lastError().text()));
            delete query;
            return false;
        }

        m_removeQuery = query;
    }

    m_removeQuery->bindValue(":uuid", data.entry.uuid.toRfc4122());
    m_removeQuery->bindValue(":mtime", data.sync.mTime.isValid() ? data.sync.mTime.toMSecsSinceEpoch()
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

bool TimeLogHistoryWorker::editEntryData(const TimeLogSyncDataEntry &data, TimeLogHistory::Fields fields)
{
    Q_ASSERT(data.entry.isValid());

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
    QString queryString = QString("UPDATE timelog SET %1 mtime=?"
                                  " WHERE uuid=?;").arg(fieldNames.isEmpty() ? QString()
                                                                             : fieldNames.join(", ") + ",");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }
    if (fields & TimeLogHistory::StartTime) {
        query.addBindValue(data.entry.startTime.toTime_t());
    }
    if (fields & TimeLogHistory::Category) {
        query.addBindValue(data.entry.category);
    }
    if (fields & TimeLogHistory::Comment) {
        query.addBindValue(data.entry.comment);
    }
    query.addBindValue(data.sync.mTime.isValid() ? data.sync.mTime.toMSecsSinceEpoch()
                                                 : QDateTime::currentMSecsSinceEpoch());
    query.addBindValue(data.entry.uuid.toRfc4122());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }

    return true;
}

bool TimeLogHistoryWorker::editEntriesCategory(const QString &oldName, const QString &newName)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("SELECT count(*) FROM timelog WHERE category=?");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }
    query.addBindValue(oldName);

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }

    query.next();
    qlonglong oldCategoryItemsCount = query.value(0).toLongLong();
    query.finish();

    if (oldCategoryItemsCount == 0) {
        return true;
    }

    queryString = QString("UPDATE timelog SET category=?, mtime=? WHERE category=?;");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }
    query.addBindValue(newName);
    query.addBindValue(QDateTime::currentMSecsSinceEpoch());
    query.addBindValue(oldName);

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }

    m_categoryRecordsCount[oldName] = 0;
    m_categoryRecordsCount[newName] += oldCategoryItemsCount;

    return true;
}

bool TimeLogHistoryWorker::addCategoryData(const TimeLogSyncDataCategory &data)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("INSERT INTO categories (uuid, category, data, mtime) VALUES(?,?,?,?);");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }
    query.addBindValue(data.category.uuid.toRfc4122());
    query.addBindValue(data.category.name);
    query.addBindValue(QJsonDocument(QJsonObject::fromVariantMap(data.category.data)).toJson(QJsonDocument::Compact));
    query.addBindValue(data.sync.mTime.isValid() ? data.sync.mTime.toMSecsSinceEpoch()
                                                 : QDateTime::currentMSecsSinceEpoch());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }

    if (!m_categories.contains(data.category.name)) {
        m_categories.insert(data.category.name, data.category);
        if (!m_categoryRecordsCount.contains(data.category.name)) {
            m_categoryRecordsCount.insert(data.category.name, 0);
        }
        updateCategories();
    } else if (!m_categories.value(data.category.name).isValid()) { // Entry-only category
        m_categories.insert(data.category.name, data.category);
        if (!data.category.data.isEmpty()) {
            updateCategories();
        }
    } else if (m_categories.value(data.category.name).data != data.category.data) {
        m_categories[data.category.name].data = data.category.data;
        updateCategories();
    }

    return true;
}

bool TimeLogHistoryWorker::removeCategoryData(const TimeLogSyncDataCategory &data)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("INSERT OR REPLACE INTO categories_removed (uuid, mtime) VALUES(?,?);");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }
    query.addBindValue(data.category.uuid.toRfc4122());
    query.addBindValue(data.sync.mTime.isValid() ? data.sync.mTime.toMSecsSinceEpoch()
                                                 : QDateTime::currentMSecsSinceEpoch());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }

    QString name(data.category.name);
    if (name.isEmpty()) {
        auto it = std::find_if(m_categories.cbegin(), m_categories.cend(),
                               [&data](const TimeLogCategory &c) {
            return c.uuid == data.category.uuid;
        });
        if (it != m_categories.cend()) {
            name = it->name;
        }
    }
    if (!name.isEmpty()) {
        if (m_categoryRecordsCount.value(name)) {
            if (!m_categories.value(name).data.isEmpty()) {
                m_categories[name].data.clear();    // Entry-only category has no data
                updateCategories();
            }
        } else {
            m_categoryRecordsCount.remove(name);
            m_categories.remove(name);
            updateCategories();
        }
    }

    return true;
}

bool TimeLogHistoryWorker::editCategoryData(const QString &oldName, const TimeLogSyncDataCategory &data)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("UPDATE categories SET category=?, data=?, mtime=? WHERE uuid=?;");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }
    query.addBindValue(data.category.name);
    query.addBindValue(QJsonDocument(QJsonObject::fromVariantMap(data.category.data)).toJson(QJsonDocument::Compact));
    query.addBindValue(data.sync.mTime.isValid() ? data.sync.mTime.toMSecsSinceEpoch()
                                                 : QDateTime::currentMSecsSinceEpoch());
    query.addBindValue(data.category.uuid.toRfc4122());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }

    m_categories.remove(oldName);
    m_categories.insert(data.category.name, data.category);
    m_categoryRecordsCount.insert(data.category.name, m_categoryRecordsCount.take(oldName));

    updateCategories();

    return true;
}

bool TimeLogHistoryWorker::syncEntryData(const QVector<TimeLogSyncDataEntry> &removed,
                                         const QVector<TimeLogSyncDataEntry> &inserted,
                                         const QVector<TimeLogSyncDataEntry> &updated,
                                         const QVector<TimeLogHistory::Fields> &updateFields)
{
    for (const TimeLogSyncDataEntry &item: removed) {
        if (!removeEntryData(item)) {
            return false;
        }
    }

    for (const TimeLogSyncDataEntry &item: inserted) {
        if (!insertEntryData(item)) {
            return false;
        }
    }

    for (int i = 0; i < updated.size(); i++) {
        if (!editEntryData(updated.at(i), updateFields.at(i))) {
            return false;
        }
    }

    return true;
}

bool TimeLogHistoryWorker::syncCategoryData(const QVector<TimeLogSyncDataCategory> &removed,
                                            const QVector<TimeLogSyncDataCategory> &inserted,
                                            const QVector<TimeLogSyncDataCategory> &updatedNew,
                                            const QVector<TimeLogSyncDataCategory> &updatedOld)
{
    for (const TimeLogSyncDataCategory &item: removed) {
        if (!removeCategoryData(item)) {
            return false;
        }
    }

    for (const TimeLogSyncDataCategory &item: inserted) {
        if (!addCategoryData(item)) {
            return false;
        }
    }

    for (int i = 0; i < updatedNew.size(); i++) {
        if (!editCategoryData(updatedOld.at(i).category.name, updatedNew.at(i))) {
            return false;
        }
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
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }
    query.addBindValue(hash);
    query.addBindValue(start.toTime_t());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }
    query.addBindValue(start.toTime_t());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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

QVector<TimeLogSyncDataEntry> TimeLogHistoryWorker::getSyncEntryData(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString where = QString(mBegin.isValid() ? "mtime >= :mBegin" : "")
                    + QString(mBegin.isValid() && mEnd.isValid() ? " AND " : "")
                    + QString(mEnd.isValid() ? "mtime <= :mEnd" : "");
    if (!where.isEmpty()) {
        where = "WHERE (" + where + ")";
    }
    QString queryString = QString("WITH result AS ( "
                                  "    SELECT uuid, start, category, comment, mtime FROM timelog %1 "
                                  "UNION ALL "
                                  "    SELECT uuid, NULL, NULL, NULL, mtime FROM timelog_removed %1 "
                                  ") "
                                  "SELECT * FROM result ORDER BY mtime ASC").arg(where);
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return QVector<TimeLogSyncDataEntry>();
    }
    if (mBegin.isValid()) {
        query.bindValue(":mBegin", mBegin.toMSecsSinceEpoch());
    }
    if (mEnd.isValid()) {
        query.bindValue(":mEnd", mEnd.toMSecsSinceEpoch());
    }

    return getSyncEntryData(query);
}

QVector<TimeLogSyncDataEntry> TimeLogHistoryWorker::getSyncEntryData(QSqlQuery &query) const
{
    QVector<TimeLogSyncDataEntry> result;

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return result;
    }

    while (query.next()) {
        TimeLogSyncDataEntry data;
        data.entry.uuid = QUuid::fromRfc4122(query.value(0).toByteArray());
        data.sync.isRemoved = query.isNull(1);
        if (!data.sync.isRemoved) { // Removed item shouldn't has valid start time
            data.entry.startTime = QDateTime::fromTime_t(query.value(1).toUInt(), Qt::UTC);
        }
        data.entry.category = query.value(2).toString();
        data.entry.comment = query.value(3).toString();
        data.sync.mTime = QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong(), Qt::UTC);

        result.append(data);
    }

    query.finish();

    return result;
}

QVector<TimeLogSyncDataCategory> TimeLogHistoryWorker::getSyncCategoryData(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString where = QString(mBegin.isValid() ? "mtime >= :mBegin" : "")
                    + QString(mBegin.isValid() && mEnd.isValid() ? " AND " : "")
                    + QString(mEnd.isValid() ? "mtime <= :mEnd" : "");
    if (!where.isEmpty()) {
        where = "WHERE (" + where + ")";
    }
    QString queryString = QString("WITH result AS ( "
                                  "    SELECT uuid, category, data, mtime FROM categories %1 "
                                  "UNION ALL "
                                  "    SELECT uuid, NULL, NULL, mtime FROM categories_removed %1 "
                                  ") "
                                  "SELECT * FROM result ORDER BY mtime ASC").arg(where);
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return QVector<TimeLogSyncDataCategory>();
    }
    if (mBegin.isValid()) {
        query.bindValue(":mBegin", mBegin.toMSecsSinceEpoch());
    }
    if (mEnd.isValid()) {
        query.bindValue(":mEnd", mEnd.toMSecsSinceEpoch());
    }

    return getSyncCategoryData(query);
}

QVector<TimeLogSyncDataCategory> TimeLogHistoryWorker::getSyncCategoryData(QSqlQuery &query) const
{
    QVector<TimeLogSyncDataCategory> result;

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return result;
    }

    while (query.next()) {
        TimeLogSyncDataCategory data;
        data.category.uuid = QUuid::fromRfc4122(query.value(0).toByteArray());
        data.category.name = query.value(1).toString();

        if (!query.value(2).isNull()) {
            QByteArray jsonString = query.value(2).toByteArray();
            QJsonParseError parseError;
            QJsonDocument document = QJsonDocument::fromJson(jsonString, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to parse category data JSON:"
                                                    << jsonString << "error at offset"
                                                    << parseError.offset
                                                    << parseError.errorString() << parseError.error;
                emit error(tr("Fail to parse category data: %1").arg(parseError.errorString()));
                return result;
            }
            data.category.data = document.object().toVariantMap();
        }

        data.sync.isRemoved = !data.category.isValid();
        data.sync.mTime = QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong(), Qt::UTC);

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
            emit error(tr("DB error: %1").arg(query->lastError().text()));
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
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return QVector<TimeLogEntry>();
    }
    query.addBindValue(category);

    return getHistory(query);
}

QVector<TimeLogSyncDataEntry> TimeLogHistoryWorker::getSyncEntriesAffected(const QUuid &uuid)
{
    if (!m_syncAffectedQuery) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery *query = new QSqlQuery(db);
        QString queryString("WITH result AS ( "
                            "    SELECT uuid, start, category, comment, mtime FROM timelog "
                            "    WHERE uuid=:uuid "
                            "UNION ALL "
                            "    SELECT uuid, NULL, NULL, NULL, mtime FROM timelog_removed "
                            "    WHERE uuid=:uuid "
                            ") "
                            "SELECT * FROM result ORDER BY mtime DESC LIMIT 1");
        if (!query->prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:"
                                                << query->lastError().text()
                                                << query->lastQuery();
            emit error(tr("DB error: %1").arg(query->lastError().text()));
            delete query;
            return QVector<TimeLogSyncDataEntry>();
        }

        m_syncAffectedQuery = query;
    }

    m_syncAffectedQuery->bindValue(":uuid", uuid.toRfc4122());

    return getSyncEntryData(*m_syncAffectedQuery);
}

QVector<TimeLogSyncDataCategory> TimeLogHistoryWorker::getSyncCategoriesAffected(const QUuid &uuid)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("WITH result AS ( "
                        "    SELECT uuid, category, data, mtime FROM categories "
                        "    WHERE uuid=:uuid "
                        "UNION ALL "
                        "    SELECT uuid, NULL, NULL, mtime FROM categories_removed "
                        "    WHERE uuid=:uuid "
                        ") "
                        "SELECT * FROM result ORDER BY mtime ASC");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return QVector<TimeLogSyncDataCategory>();
    }
    query.bindValue(":uuid", uuid.toRfc4122());

    return getSyncCategoryData(query);
}

bool TimeLogHistoryWorker::getSyncDataExists(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    QString queryString("SELECT mtime FROM (SELECT coalesce( "
                        "    (SELECT mtime FROM timelog "
                        "        WHERE mtime BETWEEN :mBegin AND :mEnd LIMIT 1), "
                        "    (SELECT mtime FROM timelog_removed "
                        "        WHERE mtime BETWEEN :mBegin AND :mEnd LIMIT 1), "
                        "    (SELECT mtime FROM categories "
                        "        WHERE mtime BETWEEN :mBegin AND :mEnd LIMIT 1), "
                        "    (SELECT mtime FROM categories_removed "
                        "        WHERE mtime BETWEEN :mBegin AND :mEnd LIMIT 1) "
                        ") AS mtime) WHERE mtime IS NOT NULL");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }
    query.bindValue(":mBegin", mBegin.toMSecsSinceEpoch());
    query.bindValue(":mEnd", mEnd.toMSecsSinceEpoch());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }

    bool result = query.next();
    query.finish();

    return result;
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
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return result;
    }
    if (maxDate.isValid()) {
        query.addBindValue(maxDate.toTime_t());
    }

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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
            emit error(tr("DB error: %1").arg(query->lastError().text()));
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
            emit error(tr("DB error: %1").arg(query->lastError().text()));
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
            emit error(tr("DB error: %1").arg(query->lastError().text()));
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
        qCDebug(HISTORY_WORKER_CATEGORY) << "Updated entries count:" << updatedData.size();
        updatedFields.insert(0, updatedData.size(), fields);
        emit dataUpdated(updatedData, updatedFields);
    }
}

bool TimeLogHistoryWorker::startTransaction(QSqlDatabase &db)
{
    if (!db.transaction()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to start transaction:" << db.lastError().text();
        emit error(tr("DB error: %1").arg(db.lastError().text()));
        return false;
    }

    return true;
}

bool TimeLogHistoryWorker::commitTransaction(QSqlDatabase &db)
{
    if (!db.commit()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to commit transaction:" << db.lastError().text();
        emit error(tr("DB error: %1").arg(db.lastError().text()));
        rollbackTransaction(db);
        return false;
    }

    return true;
}

void TimeLogHistoryWorker::rollbackTransaction(QSqlDatabase &db)
{
    if (!db.rollback()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to rollback transaction:" << db.lastError().text();
        emit error(tr("DB error: %1").arg(db.lastError().text()));
    }
}

QString TimeLogHistoryWorker::fixCategoryName(const QString &name) const
{
    return name.split(m_categorySplitRegexp, QString::SkipEmptyParts).join(" > ").trimmed();
}

bool TimeLogHistoryWorker::fetchCategories()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    // FULL OUTER JOIN
    QString queryString("WITH t AS (SELECT category, count(*) AS count FROM timelog GROUP BY category), "
                        "    c AS (SELECT uuid, category, data FROM categories) "
                        "SELECT "
                        "    c.uuid AS uuid, "
                        "    ifnull(c.category, t.category) AS category, "
                        "    ifnull(t.count, 0) AS count, "
                        "    c.data AS data "
                        "FROM t LEFT OUTER JOIN c ON t.category = c.category "
                        "UNION ALL "
                        "SELECT "
                        "    c.uuid AS uuid, "
                        "    ifnull(c.category, t.category) AS category, "
                        "    ifnull(t.count, 0) AS count, "
                        "    c.data AS data "
                        "FROM c LEFT OUTER JOIN t ON t.category = c.category WHERE t.category IS NULL");
    if (!prepareAndExecQuery(query, queryString)) {
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return false;
    }

    QMap<QString, TimeLogCategory> resultData;
    QHash<QString, int> resultRecordsCount;
    qlonglong size = 0;

    while (query.next()) {
        TimeLogCategory category;

        if (!query.value(0).isNull()) {
            category.uuid = QUuid::fromRfc4122(query.value(0).toByteArray());
        }   // Entry-only category has null uuid

        category.name = query.value(1).toString();

        int count = query.value(2).toInt();

        if (!query.value(3).isNull()) {
            QByteArray jsonString = query.value(3).toByteArray();
            QJsonParseError parseError;
            QJsonDocument document = QJsonDocument::fromJson(jsonString, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to parse category data JSON:"
                                                    << jsonString << "error at offset"
                                                    << parseError.offset
                                                    << parseError.errorString() << parseError.error;
                emit error(tr("Fail to parse category data: %1").arg(parseError.errorString()));
                return false;
            }
            category.data = document.object().toVariantMap();
        }

        resultData.insert(category.name, category);
        resultRecordsCount.insert(category.name, count);

        size += count;
    }

    query.finish();

    m_categories.swap(resultData);
    m_categoryRecordsCount.swap(resultRecordsCount);

    setSize(size);

    updateCategories();

    return true;
}

bool TimeLogHistoryWorker::checkIsDBEmpty()
{
    return (m_size == 0 && m_categories.isEmpty() && !getSyncDataExists());
}

bool TimeLogHistoryWorker::populateCategories()
{
    QVector<TimeLogCategory> defaultCategories(TimeLogDefaultCategories::defaultCategories());

    for (const TimeLogCategory &category: defaultCategories) {
        // Use 0 mTime to make edited categories be newer than populated ones
        TimeLogSyncDataCategory syncCategory(category, QDateTime::fromMSecsSinceEpoch(0, Qt::UTC));
        if (!addCategoryData(syncCategory)) {
            return false;
        }
    }

    return true;
}

void TimeLogHistoryWorker::updateCategories()
{
    m_categoryTree = parseCategories(m_categories.keys());

    emit categoriesChanged(m_categoryTree);
}

QSharedPointer<TimeLogCategoryTreeNode> TimeLogHistoryWorker::parseCategories(const QStringList &categories) const
{
    TimeLogCategoryTreeNode *rootCategory = new TimeLogCategoryTreeNode("RootCategory");

    for (const QString &category: categories) {
        QStringList categoryFields = category.split(m_categorySplitRegexp, QString::SkipEmptyParts);
        TimeLogCategoryTreeNode *parentCategory = rootCategory;
        for (const QString &categoryField: categoryFields) {
            TimeLogCategoryTreeNode *categoryObject = parentCategory->children().value(categoryField);
            if (!categoryObject) {
                categoryObject = new TimeLogCategoryTreeNode(categoryField, parentCategory);
                QString fullName(categoryObject->fullName());
                categoryObject->category = m_categories.value(categoryObject->fullName());
                categoryObject->hasItems = m_categoryRecordsCount.value(fullName) > 0;
            }
            parentCategory = categoryObject;
        }
    }

    return QSharedPointer<TimeLogCategoryTreeNode>(rootCategory);
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
                        "    SELECT mtime, uuid FROM timelog_removed "
                        "    WHERE (mtime BETWEEN :mBegin AND :mEnd) "
                        "UNION ALL "
                        "    SELECT mtime, uuid FROM categories "
                        "    WHERE (mtime BETWEEN :mBegin AND :mEnd) "
                        "UNION ALL "
                        "    SELECT mtime, uuid FROM categories_removed "
                        "    WHERE (mtime BETWEEN :mBegin AND :mEnd) "
                        ") "
                        "SELECT * FROM result ORDER BY mtime ASC, uuid ASC");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
        return result;
    }
    query.bindValue(":mBegin", begin.toMSecsSinceEpoch());
    query.bindValue(":mEnd", end.toMSecsSinceEpoch());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(tr("DB error: %1").arg(query.lastError().text()));
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
