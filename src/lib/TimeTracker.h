#ifndef TIMETRACKER_H
#define TIMETRACKER_H

#include <QObject>
#include <QVariant>
#include <QPointF>
#include <QUrl>

#include "TimeLogData.h"
#include "TimeLogStats.h"

class QQuickItem;

class TimeLogHistory;
class DataSyncer;

class TimeTracker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl dataPath MEMBER m_dataPath WRITE setDataPath NOTIFY dataPathChanged)
    Q_PROPERTY(DataSyncer* syncer MEMBER m_syncer NOTIFY syncerChanged)
    Q_PROPERTY(QStringList categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(int undoCount READ undoCount NOTIFY undoCountChanged)
public:
    explicit TimeTracker(QObject *parent = 0);
    virtual ~TimeTracker();

    void setDataPath(const QUrl &dataPathUrl);

    TimeLogHistory *history();

    QStringList categories() const;
    int undoCount() const;

    Q_INVOKABLE static TimeLogData createTimeLogData(QDateTime startTime, QString category,
                                                     QString comment);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void editCategory(QString oldName, QString newName);
    Q_INVOKABLE void getStats(const QDateTime &begin = QDateTime::fromTime_t(0),
                              const QDateTime &end = QDateTime::currentDateTime(),
                              const QString &category = QString(),
                              const QString &separator = ">");
    Q_INVOKABLE static QString durationText(int duration, int maxUnits = 7);
    Q_INVOKABLE static QPointF mapToGlobal(QQuickItem *item);

signals:
    void dataPathChanged(const QUrl &newDataPath) const;
    void historyChanged(TimeLogHistory *newHistory) const;
    void syncerChanged(DataSyncer *newSyncer) const;
    void error(const QString &errorText) const;
    void statsDataAvailable(QVariantMap data, QDateTime until) const;
    void categoriesChanged(QStringList newCategories) const;
    void undoCountChanged(int newUndoCount) const;

private slots:
    void statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const;
    void updateCategories(QSet<QString> categories);
    void updateUndoCount(int undoCount);

private:
    void setHistory(TimeLogHistory *history);
    void setSyncer(DataSyncer *syncer);

    QUrl m_dataPath;
    TimeLogHistory *m_history;
    DataSyncer *m_syncer;
    QStringList m_categories;
    int m_undoCount;
};

#endif // TIMETRACKER_H
