#ifndef TIMETRACKER_H
#define TIMETRACKER_H

#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include <QPointF>
#include <QUrl>

#include "TimeLogData.h"
#include "TimeLogCategory.h"
#include "TimeLogStats.h"

class TimeLogHistory;
class TimeLogCategoryTreeNode;
class DataSyncer;

class TimeTracker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl dataPath MEMBER m_dataPath WRITE setDataPath NOTIFY dataPathChanged)
    Q_PROPERTY(DataSyncer* syncer MEMBER m_syncer NOTIFY syncerChanged)
    Q_PROPERTY(QSharedPointer<TimeLogCategoryTreeNode> categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(int undoCount READ undoCount NOTIFY undoCountChanged)
public:
    explicit TimeTracker(QObject *parent = 0);

    void setDataPath(const QUrl &dataPathUrl);

    TimeLogHistory *history();

    QSharedPointer<TimeLogCategoryTreeNode> categories() const;
    int undoCount() const;

    Q_INVOKABLE static TimeLogData createTimeLogData(QDateTime startTime, QString category,
                                                     QString comment);
    Q_INVOKABLE static TimeLogCategoryData createTimeLogCategoryData(QString name, QVariantMap data);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void getStats(const QDateTime &begin = QDateTime::fromTime_t(0, Qt::UTC),
                              const QDateTime &end = QDateTime::currentDateTimeUtc(),
                              const QString &category = QString(),
                              const QString &separator = ">");
    Q_INVOKABLE static QString durationText(int duration, int maxUnits = 7);
    Q_INVOKABLE static QUrl documentsLocation();
    Q_INVOKABLE static bool createFolder(const QString &path, const QString &name);

    void addCategory(const TimeLogCategory &category);
    void removeCategory(const QString &name);
    void editCategory(const QString &oldName, const TimeLogCategory &category);

signals:
    void dataPathChanged(const QUrl &newDataPath) const;
    void historyChanged(TimeLogHistory *newHistory) const;
    void syncerChanged(DataSyncer *newSyncer) const;
    void error(const QString &errorText) const;
    void statsDataAvailable(QVariantMap data, QDateTime until) const;
    void categoriesChanged(const QSharedPointer<TimeLogCategoryTreeNode> newCategories) const;
    void undoCountChanged(int newUndoCount) const;

    void showSearchRequested(const QString &category) const;
    void showStatsRequested(const QString &category) const;
    void showHistoryRequested(const QDateTime &begin, const QDateTime &end) const;
    void showDialogRequested(QObject *dialog) const;
    void backRequested() const;

private slots:
    void statsDataAvailable(QVector<TimeLogStats> data, QDateTime until) const;
    void updateCategories(const QSharedPointer<TimeLogCategoryTreeNode> &categories);
    void updateUndoCount(int undoCount);

private:
    void setHistory(TimeLogHistory *history);
    void setSyncer(DataSyncer *syncer);

    QUrl m_dataPath;
    TimeLogHistory *m_history;
    DataSyncer *m_syncer;
    QSharedPointer<TimeLogCategoryTreeNode> m_categories;
    int m_undoCount;
};

#endif // TIMETRACKER_H
