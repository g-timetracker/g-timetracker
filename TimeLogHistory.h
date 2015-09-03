#ifndef TIMELOGHISTORY_H
#define TIMELOGHISTORY_H

#include <QObject>
#include <QSqlQuery>

#include "TimeLogEntry.h"

class TimeLogHistory : public QObject
{
    Q_OBJECT
public:
    explicit TimeLogHistory(QObject *parent = 0);
    ~TimeLogHistory();

    bool init();
    qlonglong size() const;

public slots:
    void insert(const TimeLogEntry &data);
    void remove(const QUuid &uuid);
    void edit(const TimeLogEntry &data);

public:
    QVector<QString> categories(const QDateTime &begin = QDateTime::fromTime_t(0),
                                const QDateTime &end = QDateTime::currentDateTime()) const;

public slots:
    void getHistory(const QDateTime &begin = QDateTime::fromTime_t(0),
                    const QDateTime &end = QDateTime::currentDateTime(),
                    const QString &category = QString()) const;
    void getHistory(const uint limit,
                    const QDateTime &until = QDateTime::currentDateTime()) const;

signals:
    void error(const QString &errorText) const;
    void dataAvailable(QVector<TimeLogEntry> data) const;

private:
    bool m_isInitialized;
    qlonglong m_size;

    QVector<TimeLogEntry> getHistory(QSqlQuery &query) const;
    bool updateSize();
};

#endif // TIMELOGHISTORY_H
