#ifndef TIMELOG_H
#define TIMELOG_H

#include <QObject>
#include <QVariant>
#include <QPointF>

#include "TimeLogData.h"
#include "TimeLogStats.h"

class QQuickItem;

class TimeLog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList categories READ categories NOTIFY categoriesChanged)
protected:
    explicit TimeLog(QObject *parent = 0);

public:
    virtual ~TimeLog();

    static TimeLog *instance();

    QStringList categories();

    Q_INVOKABLE static TimeLogData createTimeLogData(QDateTime startTime, int durationTime,
                                                     QString category, QString comment);
    Q_INVOKABLE static void editCategory(QString oldName, QString newName);
    Q_INVOKABLE static void getStats(const QDateTime &begin = QDateTime::fromTime_t(0),
                                     const QDateTime &end = QDateTime::currentDateTime(),
                                     const QString &category = QString());
    Q_INVOKABLE static QPointF mapToGlobal(QQuickItem *item);

signals:
    void error(const QString &errorText) const;
    void statsDataAvailable(QVariantMap data, QDateTime until) const;
    void categoriesChanged(QStringList) const;

private slots:
    void statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const;
    void categoriesAvailable(QSet<QString> categories) const;
};

#endif // TIMELOG_H
