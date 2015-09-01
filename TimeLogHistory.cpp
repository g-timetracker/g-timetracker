#include <QStandardPaths>
#include <QDir>
#include <QSqlError>

#include <QLoggingCategory>

#include "TimeLogHistory.h"

Q_LOGGING_CATEGORY(TIME_LOG_HISTORY_CATEGORY, "TimeLogHistory", QtInfoMsg)

TimeLogHistory::TimeLogHistory(QObject *parent) :
    QObject(parent)
{
    QString path(QString("%1/timelog")
                 .arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation)));
    QDir().mkpath(path);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "timelog");
    db.setDatabaseName(QString("%1/%2.sqlite").arg(path).arg("db"));

    if (!db.open()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to open db:" << db.lastError();
        return;
    }

    QSqlQuery query(db);
    QString queryString("CREATE TABLE IF NOT EXISTS timelog"
                        " (uuid BLOB UNIQUE, start INTEGER PRIMARY KEY, category TEXT, comment TEXT, mtime INTEGER);");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError();
        return;
    }

    if (!query.exec()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to execute query:" << query.lastError();
        return;
    }
}

TimeLogHistory::~TimeLogHistory()
{
    QSqlDatabase::database("timelog").close();
    QSqlDatabase::removeDatabase("timelog");
}

void TimeLogHistory::insert(const TimeLogEntry &data)
{
    Q_ASSERT(data.isValid());

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
}

void TimeLogHistory::remove(const QUuid &uuid)
{
    Q_ASSERT(!uuid.isNull());

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
}

void TimeLogHistory::edit(const TimeLogEntry &data)
{
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

bool TimeLogHistory::hasHistory(const QDateTime &before) const
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("SELECT uuid, start, category, comment FROM timelog"
                        " WHERE start < ? ORDER BY start DESC LIMIT 1");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        emit error(query.lastError().text());
        return false;
    }
    query.addBindValue(before.toTime_t());

    if (!query.exec()) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to execute query:" << query.lastError().text();
        emit error(query.lastError().text());
        return false;
    }

    bool result = query.next();

    query.finish();

    return result;
}

QVector<TimeLogEntry> TimeLogHistory::getHistory(const QDateTime &begin, const QDateTime &end, const QString &category) const
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString = QString("SELECT uuid, start, category, comment FROM timelog"
                                  " WHERE (start BETWEEN ? AND ?) %1 ORDER BY start")
                                  .arg(category.isEmpty() ? "" : "AND category=?");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        emit error(query.lastError().text());
        return QVector<TimeLogEntry>();
    }
    query.addBindValue(begin.toTime_t());
    query.addBindValue(end.toTime_t());
    if (!category.isEmpty()) {
        query.addBindValue(category);
    }

    return getHistory(query);
}

QVector<TimeLogEntry> TimeLogHistory::getHistory(const uint limit, const QDateTime &until) const
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("SELECT uuid, start, category, comment FROM timelog"
                        " WHERE start < ? ORDER BY start DESC LIMIT ?");
    if (!query.prepare(queryString)) {
        qCCritical(TIME_LOG_HISTORY_CATEGORY) << "Fail to prepare query:" << query.lastError().text();
        emit error(query.lastError().text());
        return QVector<TimeLogEntry>();
    }
    query.addBindValue(until.toTime_t());
    query.addBindValue(limit);

    QVector<TimeLogEntry> result = getHistory(query);
    std::reverse(result.begin(), result.end());
    return result;
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
