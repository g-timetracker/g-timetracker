#include <QStandardPaths>
#include <QDir>
#include <QSqlError>

#include <QLoggingCategory>

#include "TimeLogHistoryWorker.h"

Q_LOGGING_CATEGORY(HISTORY_WORKER_CATEGORY, "TimeLogHistoryWorker", QtInfoMsg)

TimeLogHistoryWorker::TimeLogHistoryWorker(QObject *parent) :
    QObject(parent),
    m_isInitialized(false),
    m_size(0)
{

}

TimeLogHistoryWorker::~TimeLogHistoryWorker()
{
    QSqlDatabase::database("timelog").close();
    QSqlDatabase::removeDatabase("timelog");
}

bool TimeLogHistoryWorker::init()
{
    QString path(QString("%1/timelog")
                 .arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation)));
    QDir().mkpath(path);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "timelog");
    db.setDatabaseName(QString("%1/%2.sqlite").arg(path).arg("db"));

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

    if (!updateCategories(QDateTime::fromTime_t(0), QDateTime::currentDateTime())) {
        return false;
    }

    m_isInitialized = true;

    return true;
}

qlonglong TimeLogHistoryWorker::size() const
{
    return m_size;
}

QSet<QString> TimeLogHistoryWorker::categories() const
{
    return m_categories;
}

void TimeLogHistoryWorker::insert(const TimeLogEntry &data)
{
    if (!m_isInitialized) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "db is not initialized";
        return;
    }

    if (insertData(data)) {
        emit dataInserted(QVector<TimeLogEntry>() << data);
    }

    return;
}

void TimeLogHistoryWorker::insert(const QVector<TimeLogEntry> &data)
{
    if (!m_isInitialized) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "db is not initialized";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    if (!db.transaction()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to start transaction:" << db.lastError().text();
        emit error(db.lastError().text());
        return;
    }

    foreach (const TimeLogEntry &entry, data) {
        if (!insertData(entry)) {
            if (!db.rollback()) {
                qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to rollback transaction:" << db.lastError().text();
                emit error(db.lastError().text());
            }

            return;
        }
    }

    if (!db.commit()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to commit transaction:" << db.lastError().text();
        emit error(db.lastError().text());
        if (!db.rollback()) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to rollback transaction:" << db.lastError().text();
            emit error(db.lastError().text());
        }
        return;
    }

    emit dataInserted(data);
}

void TimeLogHistoryWorker::remove(const TimeLogEntry &data)
{
    Q_ASSERT(data.isValid());

    if (!m_isInitialized) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "db is not initialized";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("DELETE FROM timelog WHERE uuid=?;");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return;
    }
    query.addBindValue(data.uuid.toRfc4122());

    if (!query.exec()) {
        qCWarning(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                           << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return;
    }

    setSize(m_size - query.numRowsAffected());

    queryString = "SELECT uuid, start, category, comment, duration FROM timelog"
                  " WHERE start < ? ORDER BY start DESC LIMIT 1";
    notifyUpdates(queryString, QVector<QDateTime>() << data.startTime);
}

void TimeLogHistoryWorker::edit(const TimeLogEntry &data, TimeLogHistory::Fields fields)
{
    if (!m_isInitialized) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "db is not initialized";
        return;
    }

    if (fields == TimeLogHistory::NoFields) {
        qCWarning(HISTORY_WORKER_CATEGORY) << "No fields specified";
        return;
    }

    QDateTime oldStart;

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    if (fields & TimeLogHistory::StartTime) {
        QString queryString("SELECT start FROM timelog WHERE uuid=?");
        if (!query.prepare(queryString)) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                                << query.lastQuery();
            emit error(query.lastError().text());
            return;
        }
        query.addBindValue(data.uuid.toRfc4122());

        if (!query.exec()) {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                                << query.executedQuery() << query.boundValues();
            emit error(query.lastError().text());
            return;
        }

        if (query.next()) {
            oldStart = QDateTime::fromTime_t(query.value(0).toUInt());
        } else {
            qCCritical(HISTORY_WORKER_CATEGORY) << "Item to update not found:\n"
                                                << data.startTime << data.category << data.uuid;
        }

        query.finish();
    }
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
    QString queryString = QString("UPDATE timelog SET %1, mtime=strftime('%s','now')"
                                  " WHERE uuid=?;").arg(fieldNames.join(", "));
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return;
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
    query.addBindValue(data.uuid.toRfc4122());

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return;
    }

    if (fields & TimeLogHistory::Category) {
        addToCategories(data.category);
    }

    if (fields & TimeLogHistory::StartTime) {
        queryString = "SELECT * FROM ( "
                      "    SELECT uuid, start, category, comment, duration FROM timelog "
                      "    WHERE start <= ? ORDER BY start DESC LIMIT 2 "
                      ") "
                      "UNION "
                      "SELECT * FROM ( "
                      "    SELECT uuid, start, category, comment, duration FROM timelog "
                      "    WHERE start < ? ORDER BY start DESC LIMIT 1 "
                      ")";
        notifyUpdates(queryString, QVector<QDateTime>() << data.startTime << oldStart);
    }
}

void TimeLogHistoryWorker::getHistory(const QDateTime &begin, const QDateTime &end, const QString &category) const
{
    if (!m_isInitialized) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "db is not initialized";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString = QString("SELECT uuid, start, category, comment, duration FROM timelog"
                                  " WHERE (start BETWEEN ? AND ?) %1 ORDER BY start")
                                  .arg(category.isEmpty() ? "" : "AND category=?");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return;
    }
    query.addBindValue(begin.toTime_t());
    query.addBindValue(end.toTime_t());
    if (!category.isEmpty()) {
        query.addBindValue(category);
    }

    QVector<TimeLogEntry> result = getHistory(query);
    if (!result.isEmpty()) {
        emit dataAvailable(result, end);
    }
}

void TimeLogHistoryWorker::getHistory(const uint limit, const QDateTime &until) const
{
    if (!m_isInitialized) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "db is not initialized";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("SELECT uuid, start, category, comment, duration FROM timelog"
                        " WHERE start < ? ORDER BY start DESC LIMIT ?");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return;
    }
    query.addBindValue(until.toTime_t());
    query.addBindValue(limit);

    QVector<TimeLogEntry> result = getHistory(query);
    if (!result.isEmpty()) {
        std::reverse(result.begin(), result.end());
        emit dataAvailable(result, until);
    }
}

bool TimeLogHistoryWorker::setupTable()
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
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

    return true;
}

bool TimeLogHistoryWorker::setupTriggers()
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString;

    queryString = "CREATE TRIGGER IF NOT EXISTS insert_item AFTER INSERT ON timelog "
                  "BEGIN "
                  "    UPDATE timelog SET duration=(NEW.start - start) "
                  "    WHERE start=( "
                  "        SELECT start FROM timelog WHERE start < NEW.start ORDER BY start DESC LIMIT 1 "
                  "    ); "
                  "    UPDATE timelog SET duration=IFNULL( "
                  "        ( SELECT start FROM timelog WHERE start > NEW.start ORDER BY start ASC LIMIT 1 ) - NEW.start, "
                  "        -1 "
                  "    ) WHERE start=NEW.start; "
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

    queryString = "CREATE TRIGGER IF NOT EXISTS delete_item AFTER DELETE ON timelog "
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

    queryString = "CREATE TRIGGER IF NOT EXISTS update_item AFTER UPDATE OF start ON timelog "
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

void TimeLogHistoryWorker::addToCategories(QString category)
{
    if (m_categories.contains(category)) {
        return;
    }

    m_categories.insert(category);

    emit categoriesChanged(m_categories);
}

bool TimeLogHistoryWorker::insertData(const TimeLogEntry &data)
{
    Q_ASSERT(data.isValid());

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("INSERT INTO timelog (uuid, start, category, comment, mtime)"
                        " VALUES (?,?,?,?,strftime('%s','now'));");
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return false;
    }
    query.addBindValue(data.uuid.toRfc4122());
    query.addBindValue(data.startTime.toTime_t());
    query.addBindValue(data.category);
    query.addBindValue(data.comment);

    if (!query.exec()) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to execute query:" << query.lastError().text()
                                            << query.executedQuery() << query.boundValues();
        emit error(query.lastError().text());
        return false;
    }

    setSize(m_size + query.numRowsAffected());
    addToCategories(data.category);

    queryString = "SELECT uuid, start, category, comment, duration FROM timelog"
                  " WHERE start <= ? ORDER BY start DESC LIMIT 2";
    if (!notifyUpdates(queryString, QVector<QDateTime>() << data.startTime)) {
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
        data.startTime = QDateTime::fromTime_t(query.value(1).toUInt());
        data.category = query.value(2).toString();
        data.comment = query.value(3).toString();
        data.durationTime = query.value(4).toInt();

        result.append(data);
    }

    query.finish();

    return result;
}

bool TimeLogHistoryWorker::notifyUpdates(const QString &queryString, const QVector<QDateTime> &values) const
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    if (!query.prepare(queryString)) {
        qCCritical(HISTORY_WORKER_CATEGORY) << "Fail to prepare query:" << query.lastError().text()
                                            << query.lastQuery();
        emit error(query.lastError().text());
        return false;
    }
    foreach (const QDateTime &value, values) {
        query.addBindValue(value.toTime_t());
    }

    QVector<TimeLogEntry> updated = getHistory(query);
    QVector<TimeLogHistory::Fields> fields;

    if (!updated.isEmpty()) {
        qCDebug(HISTORY_WORKER_CATEGORY) << "Updated items count:" << updated.size();
        fields.insert(0, updated.size(), TimeLogHistory::DurationTime);
        emit dataUpdated(updated, fields);
    }

    return true;
}

bool TimeLogHistoryWorker::updateSize()
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
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
    QSqlDatabase db = QSqlDatabase::database("timelog");
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

    m_categories.swap(result);
    emit categoriesChanged(m_categories);

    return true;
}
