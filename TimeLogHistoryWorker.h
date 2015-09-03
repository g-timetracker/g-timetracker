#ifndef TIMELOGHISTORYWORKER_H
#define TIMELOGHISTORYWORKER_H

#include <QObject>
#include <QSqlQuery>
#include <QSet>

#include "TimeLogEntry.h"

class TimeLogHistoryWorker : public QObject
{
    Q_OBJECT
public:
    explicit TimeLogHistoryWorker(QObject *parent = 0);
    ~TimeLogHistoryWorker();

    bool init();
    qlonglong size() const;
    QSet<QString> categories() const;

public slots:
    void insert(const TimeLogEntry &data);
    void remove(const QUuid &uuid);
    void edit(const TimeLogEntry &data);

    void getHistory(const QDateTime &begin = QDateTime::fromTime_t(0),
                    const QDateTime &end = QDateTime::currentDateTime(),
                    const QString &category = QString()) const;
    void getHistory(const uint limit,
                    const QDateTime &until = QDateTime::currentDateTime()) const;

signals:
    void error(const QString &errorText) const;
    void dataAvailable(QVector<TimeLogEntry> data) const;

    void sizeChanged(qlonglong size) const;
    void categoriesChanged(QSet<QString> categories) const;

private:
    bool m_isInitialized;
    qlonglong m_size;
    QSet<QString> m_categories;

    void setSize(qlonglong size);
    void addToCategories(QString category);

    QVector<TimeLogEntry> getHistory(QSqlQuery &query) const;
    bool updateSize();
    bool updateCategories(const QDateTime &begin = QDateTime::fromTime_t(0),
                          const QDateTime &end = QDateTime::currentDateTime());
};

#endif // TIMELOGHISTORYWORKER_H
