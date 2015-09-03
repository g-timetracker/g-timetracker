#include <QStandardPaths>
#include <QDir>
#include <QSqlError>

#include <QLoggingCategory>

#include "TimeLogHistory.h"

Q_LOGGING_CATEGORY(TIME_LOG_HISTORY_CATEGORY, "TimeLogHistory", QtInfoMsg)

TimeLogHistory::TimeLogHistory(QObject *parent) :
    QObject(parent),
    m_isInitialized(false),
    m_size(0)
{

}

TimeLogHistory::~TimeLogHistory()
{
    QSqlDatabase::database("timelog").close();
    QSqlDatabase::removeDatabase("timelog");
}

bool TimeLogHistory::init()
{
    QString path(QString("%1/timelog")
                 .arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation)));
    QDir().mkpath(path);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "timelog");
    db.setDatabaseName(QString("%1/%2.sqlite").arg(path).arg("db"));

    if (!db.open()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to open db:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    QString queryString("CREATE TABLE IF NOT EXISTS timelog"
                        " (uuid BLOB UNIQUE, start INTEGER PRIMARY KEY, category TEXT, comment TEXT, mtime INTEGER);");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        return false;
    }

    if (!query.exec()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to execute query:" << query.lastError().text();
        return false;
    }

    if (!updateSize()) {
        return false;
    }

    m_isInitialized = true;

    return true;
}

qlonglong TimeLogHistory::size() const
{
    return m_size;
}

void TimeLogHistory::insert(const TimeLogEntry &data)
{
    Q_ASSERT(data.isValid());

    if (!m_isInitialized) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "db is not initialized";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("INSERT INTO timelog (uuid, start, category, comment, mtime)"
                        " VALUES (?,?,?,?,strftime('%s','now'));");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        emit error(query.lastError().text());
        return;
    }
    query.addBindValue(data.uuid.toRfc4122());
    query.addBindValue(data.startTime.toTime_t());
    query.addBindValue(data.category);
    query.addBindValue(data.comment);

    if (!query.exec()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to execute query:" << query.lastError().text();
        emit error(query.lastError().text());
        return;
    }

    m_size += query.numRowsAffected();
}

void TimeLogHistory::remove(const QUuid &uuid)
{
    Q_ASSERT(!uuid.isNull());

    if (!m_isInitialized) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "db is not initialized";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("DELETE FROM timelog WHERE uuid=?;");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        emit error(query.lastError().text());
        return;
    }
    query.addBindValue(uuid.toRfc4122());

    if (!query.exec()) {
        qCWarning(TIME_LOG_HISTORY_CATEGORY) << "Fail to execute query:" << query.lastError().text();
        emit error(query.lastError().text());
        return;
    }

    m_size -= query.numRowsAffected();
}

void TimeLogHistory::edit(const TimeLogEntry &data)
{
    if (!m_isInitialized) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "db is not initialized";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("UPDATE timelog SET start=?, category=?, comment=?, mtime=strftime('%s','now')"
                        " WHERE uuid=?;");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        emit error(query.lastError().text());
        return;
    }
    query.addBindValue(data.startTime.toTime_t());
    query.addBindValue(data.category);
    query.addBindValue(data.comment);
    query.addBindValue(data.uuid.toRfc4122());

    if (!query.exec()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to execute query:" << query.lastError().text();
        emit error(query.lastError().text());
        return;
    }
}

QVector<QString> TimeLogHistory::categories(const QDateTime &begin, const QDateTime &end) const
{
    if (!m_isInitialized) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "db is not initialized";
        return QVector<QString>();
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("SELECT DISTINCT category FROM timelog"
                        " WHERE start BETWEEN ? AND ? ORDER BY category");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        emit error(query.lastError().text());
        return QVector<QString>();
    }
    query.addBindValue(begin.toTime_t());
    query.addBindValue(end.toTime_t());

    QVector<QString> result;

    if (!query.exec()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to execute query:" << query.lastError().text();
        emit error(query.lastError().text());
        return QVector<QString>();
    }

    while (query.next()) {
        result.append(query.value(0).toString());
    }

    query.finish();

    return result;
}

void TimeLogHistory::getHistory(const QDateTime &begin, const QDateTime &end, const QString &category) const
{
    if (!m_isInitialized) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "db is not initialized";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString = QString("SELECT uuid, start, category, comment FROM timelog"
                                  " WHERE (start BETWEEN ? AND ?) %1 ORDER BY start")
                                  .arg(category.isEmpty() ? "" : "AND category=?");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
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
        emit dataAvailable(result);
    }
}

void TimeLogHistory::getHistory(const uint limit, const QDateTime &until) const
{
    if (!m_isInitialized) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "db is not initialized";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("SELECT uuid, start, category, comment FROM timelog"
                        " WHERE start < ? ORDER BY start DESC LIMIT ?");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        emit error(query.lastError().text());
        return;
    }
    query.addBindValue(until.toTime_t());
    query.addBindValue(limit);

    QVector<TimeLogEntry> result = getHistory(query);
    if (!result.isEmpty()) {
        std::reverse(result.begin(), result.end());
        emit dataAvailable(result);
    }
}

QVector<TimeLogEntry> TimeLogHistory::getHistory(QSqlQuery &query) const
{
    QVector<TimeLogEntry> result;

    if (!query.exec()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to execute query:" << query.lastError().text();
        emit error(query.lastError().text());
        return result;
    }

    while (query.next()) {
        TimeLogEntry data;
        data.uuid = QUuid::fromRfc4122(query.value(0).toByteArray());
        data.startTime = QDateTime::fromTime_t(query.value(1).toUInt());
        data.category = query.value(2).toString();
        data.comment = query.value(3).toString();

        result.append(data);
    }

    query.finish();

    return result;
}

bool TimeLogHistory::updateSize()
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString = QString("SELECT count(*) FROM timelog");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        emit error(query.lastError().text());
        return false;
    }

    if (!query.exec()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to execute query:" << query.lastError().text();
        emit error(query.lastError().text());
        return false;
    }

    query.next();
    m_size = query.value(0).toULongLong();
    query.finish();

    return true;
}
