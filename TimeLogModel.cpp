#include <QLoggingCategory>

#include "TimeLogModel.h"
#include "TimeLogHistory.h"

Q_LOGGING_CATEGORY(TIME_LOG_MODEL_CATEGORY, "TimeLogModel", QtInfoMsg)

static const int defaultPopulateCount(5);

bool startTimeCompare(const TimeLogEntry &a, const TimeLogEntry &b)
{
    return a.startTime < b.startTime;
}

TimeLogModel::TimeLogModel(TimeLogHistory *history, QObject *parent) :
    SUPER(parent),
    m_history(history)
{
    connect(m_history, SIGNAL(error(QString)),
            this, SLOT(historyError(QString)));
    connect(m_history, SIGNAL(dataAvailable(QVector<TimeLogEntry>,QDateTime)),
            this, SLOT(historyDataAvailable(QVector<TimeLogEntry>,QDateTime)));
}

int TimeLogModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_timeLog.size();
}

bool TimeLogModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent != QModelIndex()) {
        return false;
    }

    return m_history->size() > m_timeLog.size();
}

void TimeLogModel::fetchMore(const QModelIndex &parent)
{
    if (parent != QModelIndex()) {
        return;
    }

    getMoreHistory();
}

QVariant TimeLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case StartTimeRole:
        return QVariant::fromValue(m_timeLog[index.row()].startTime);
    case DurationTimeRole:
        return QVariant::fromValue(m_timeLog[index.row()].durationTime);
    case CategoryRole:
        return QVariant::fromValue(m_timeLog[index.row()].category);
    case CommentRole:
        return QVariant::fromValue(m_timeLog[index.row()].comment);
    case Qt::DisplayRole:
        return QVariant::fromValue(QString("%1 | %2").arg(m_timeLog[index.row()].startTime.toString()).arg(m_timeLog[index.row()].category));
    default:
        return QVariant();
    }
}

QVariant TimeLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return QString("data");
    } else {
        return section;
    }
}

QHash<int, QByteArray> TimeLogModel::roleNames() const
{
    QHash<int, QByteArray> roles = SUPER::roleNames();
    roles[StartTimeRole] = "startTime";
    roles[DurationTimeRole] = "durationTime";
    roles[CategoryRole] = "category";
    roles[CommentRole] = "comment";

    return roles;
}

Qt::ItemFlags TimeLogModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return SUPER::flags(index) | Qt::ItemIsEditable; // | Qt::ItemIsSelectable;
}

bool TimeLogModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    switch (role) {
    case StartTimeRole: {
        QDateTime time = value.toDateTime();
        if (!time.isValid()) {
            return false;
        }
        m_timeLog[index.row()].startTime = time;
        recalcDuration(index.parent(), index.row(), index.row());
        m_history->edit(m_timeLog.at(index.row()), TimeLogHistory::StartTime);
        break;
    }
    case DurationTimeRole:
        return false;   // This property can only be calculated
    case CategoryRole:
        m_timeLog[index.row()].category = value.toString();
        m_history->edit(m_timeLog.at(index.row()), TimeLogHistory::Category);
        break;
    case CommentRole:
        m_timeLog[index.row()].comment = value.toString();
        m_history->edit(m_timeLog.at(index.row()), TimeLogHistory::Comment);
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, QVector<int>() << role);

    return true;
}

bool TimeLogModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent != QModelIndex()) {
        return false;
    }

    QVector<TimeLogEntry> removed = m_timeLog.mid(row, count);

    beginRemoveRows(parent, row, row + count - 1);
    m_timeLog.remove(row, count);
    endRemoveRows();

    if (row != 0) {   // No need to recalculate, if items removed at the beginning
        recalcDuration(parent, row-1, row-1);
    }

    foreach (const TimeLogEntry &entry, removed) {
        m_history->remove(entry.uuid);
    }

    return true;
}

TimeLogData TimeLogModel::timeLogData(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return TimeLogData();
    }

    return static_cast<TimeLogData>(m_timeLog.at(index.row()));
}

void TimeLogModel::appendItem(TimeLogData data)
{
    TimeLogEntry entry(QUuid::createUuid(), data);

    int itemIndex = m_timeLog.size();
    beginInsertRows(QModelIndex(), itemIndex, itemIndex);
    m_timeLog.append(entry);
    endInsertRows();

    recalcDuration(QModelIndex(), itemIndex, itemIndex);

    m_history->insert(entry);
}

void TimeLogModel::insertItem(const QModelIndex &index, TimeLogData data)
{
    TimeLogEntry entry(QUuid::createUuid(), data);

    beginInsertRows(index.parent(), index.row(), index.row());
    m_timeLog.insert(index.row(), entry);
    endInsertRows();

    recalcDuration(index.parent(), index.row(), index.row());

    m_history->insert(entry);
}

void TimeLogModel::historyError(const QString &errorText)
{
    emit error(QString("Database error: %1").arg(errorText));
    beginResetModel();
    m_timeLog.clear();
    m_requestedData.clear();
    endResetModel();
}

void TimeLogModel::historyDataAvailable(QVector<TimeLogEntry> data, QDateTime until)
{
    if (!m_requestedData.contains(until)) {
        qCDebug(TIME_LOG_MODEL_CATEGORY) << "Discarding received but not requested data for time"
                                         << until;
        return;
    }

    int index = 0;

    if (!m_timeLog.isEmpty() && !startTimeCompare(data.last(), m_timeLog.first())) {
        QVector<TimeLogEntry>::iterator it = std::lower_bound(m_timeLog.begin(), m_timeLog.end(),
                                                              data.last(), startTimeCompare);
        index = it - m_timeLog.begin();
    }

    beginInsertRows(QModelIndex(), index, index + data.size() - 1);
    if (index == 0) {
        data.append(m_timeLog);
        m_timeLog.swap(data);
    } else {
        qCWarning(TIME_LOG_MODEL_CATEGORY) << "Inserting data not into beginning, current data:\n"
                                           << m_timeLog.first().startTime << "-" << m_timeLog.last().startTime
                                           << "\nnew data:\n"
                                           << data.first().startTime << "-" << data.last().startTime;
        m_timeLog.insert(index, data.size(), TimeLogEntry());
        for (int i = 0; i < data.size(); i++) {
            m_timeLog[index+i] = data.at(i);
        }
    }
    endInsertRows();

    m_requestedData.remove(until);
}

void TimeLogModel::getMoreHistory()
{
    QDateTime until = m_timeLog.size() ? m_timeLog.at(0).startTime : QDateTime::currentDateTime();
    if (m_requestedData.contains(until)) {
        qCDebug(TIME_LOG_MODEL_CATEGORY) << "Alredy requested data for time" << until;
        return;
    }
    m_requestedData.insert(until);
    m_history->getHistory(defaultPopulateCount, until);
}

void TimeLogModel::recalcDuration(const QModelIndex &parent, int first, int last)
{
    // Update duration time for preceeding item, if it exists
    int start = (first == 0) ? first : first - 1;
    int end = (last == m_timeLog.size() - 1) ? last - 1 : last;

    for (int i = start; i <= end; i++) {
        m_timeLog[i].durationTime = m_timeLog.at(i).startTime.secsTo(m_timeLog.at(i+1).startTime);
    }

    // Duration for most recent item calculated up to current time
    if (last == m_timeLog.size() - 1) {
        m_timeLog[last].durationTime = -1;
    }

    emit dataChanged(index(start, 0, parent), index(last, 0, parent),
                     QVector<int>() << DurationTimeRole);
}
