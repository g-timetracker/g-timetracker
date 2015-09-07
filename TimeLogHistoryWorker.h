#ifndef TIMELOGHISTORYWORKER_H
#define TIMELOGHISTORYWORKER_H

#include <QObject>
#include <QSqlQuery>
#include <QSet>

#include "TimeLogEntry.h"
#include "TimeLogHistory.h"

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
    bool insert(const TimeLogEntry &data);
    bool insert(const QVector<TimeLogEntry> &data);
    void remove(const QUuid &uuid);
    void edit(const TimeLogEntry &data, TimeLogHistory::Fields fields);

    void getHistory(const QDateTime &begin = QDateTime::fromTime_t(0),
                    const QDateTime &end = QDateTime::currentDateTime(),
                    const QString &category = QString()) const;
    void getHistory(const uint limit,
                    const QDateTime &until = QDateTime::currentDateTime()) const;

signals:
    void error(const QString &errorText) const;
    void dataAvailable(QVector<TimeLogEntry> data, QDateTime until) const;

    void sizeChanged(qlonglong size) const;
    void categoriesChanged(QSet<QString> categories) const;

private:
    bool m_isInitialized;
    qlonglong m_size;
    QSet<QString> m_categories;

    bool setupTable();
    bool setupTriggers();
    void setSize(qlonglong size);
    void addToCategories(QString category);

    QVector<TimeLogEntry> getHistory(QSqlQuery &query) const;
    bool updateSize();
    bool updateCategories(const QDateTime &begin = QDateTime::fromTime_t(0),
                          const QDateTime &end = QDateTime::currentDateTime());
};

#endif // TIMELOGHISTORYWORKER_H
