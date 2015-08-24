#include <QStandardPaths>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>

#include "TimeLogHistory.h"

TimeLogHistory::TimeLogHistory(QObject *parent) :
    QObject(parent)
{
    QString path(QString("%1/timelog")
                 .arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation)));
    QDir().mkpath(path);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "timelog");
    db.setDatabaseName(QString("%1/%2.sqlite").arg(path).arg("db"));

    if (!db.open()) {
        qWarning() << "Fail to open db" << db.lastError();
        return;
    }

    QSqlQuery query(db);
    QString queryString("CREATE TABLE IF NOT EXISTS timelog"
                        " (uuid BLOB UNIQUE, start INTEGER PRIMARY KEY, category TEXT, comment TEXT, mtime INTEGER);");
    if (!query.prepare(queryString)) {
        qWarning() << "Fail to execute query" << query.lastError();
        return;
    }

    query.exec();
}

TimeLogHistory::~TimeLogHistory()
{
    QSqlDatabase::database("timelog").close();
    QSqlDatabase::removeDatabase("timelog");
}

void TimeLogHistory::insert(const TimeLogEntry &data)
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("INSERT INTO timelog (uuid, start, category, comment, mtime)"
                        " VALUES (?,?,?,?,strftime('%s','now'));");
    if (!query.prepare(queryString)) {
        qWarning() << "Fail to execute query" << query.lastError();
        return;
    }
    query.addBindValue(data.uuid.toRfc4122());
    query.addBindValue(data.startTime.toTime_t());
    query.addBindValue(data.category);
    query.addBindValue(data.comment);

    query.exec();
}

void TimeLogHistory::remove(const QUuid &uuid)
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("DELETE FROM timelog WHERE uuid=?;");
    if (!query.prepare(queryString)) {
        qWarning() << "Fail to execute query" << query.lastError();
        return;
    }
    query.addBindValue(uuid.toRfc4122());

    query.exec();
}

void TimeLogHistory::edit(const TimeLogEntry &data)
{
    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("UPDATE timelog SET start=?, category=?, comment=?, mtime=strftime('%s','now')"
                        " WHERE uuid=?;");
    if (!query.prepare(queryString)) {
        qWarning() << "Fail to execute query" << query.lastError();
        return;
    }
    query.addBindValue(data.startTime.toTime_t());
    query.addBindValue(data.category);
    query.addBindValue(data.comment);
    query.addBindValue(data.uuid.toRfc4122());

    query.exec();
}

QVector<TimeLogEntry> TimeLogHistory::getHistory(const QDateTime &begin, const QDateTime &end) const
{
    QVector<TimeLogEntry> result;

    QSqlDatabase db = QSqlDatabase::database("timelog");
    QSqlQuery query(db);
    QString queryString("SELECT uuid, start, category, comment FROM timelog"
                        " WHERE start BETWEEN ? AND ? ORDER BY start");
    if (!query.prepare(queryString)) {
        qWarning() << "Fail to execute query" << query.lastError();
        return result;
    }
    query.addBindValue(begin.toTime_t());
    query.addBindValue(end.toTime_t());

    query.exec();

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
