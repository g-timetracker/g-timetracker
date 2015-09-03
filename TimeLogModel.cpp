#include "TimeLogModel.h"
#include "TimeLogHistory.h"

static const int defaultPopulateCount(5);

TimeLogModel::TimeLogModel(TimeLogHistory *history, QObject *parent) :
    SUPER(parent),
    m_history(history)
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(processRowsInserted(QModelIndex,int,int)), Qt::QueuedConnection);
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(processRowsRemoved(QModelIndex,int,int)), Qt::QueuedConnection);
    connect(m_history, SIGNAL(error(QString)),
            this, SLOT(historyError(QString)));
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

    return m_history->hasHistory(m_timeLog.size() ? m_timeLog.at(0).startTime : QDateTime::currentDateTime());;
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
        QMetaObject::invokeMethod(m_history, "edit", Qt::QueuedConnection,
                                  Q_ARG(TimeLogEntry, m_timeLog.at(index.row())));
        break;
    }
    case DurationTimeRole:
        return false;   // This property can only be calculated
    case CategoryRole:
        m_timeLog[index.row()].category = value.toString();
        QMetaObject::invokeMethod(m_history, "edit", Qt::QueuedConnection,
                                  Q_ARG(TimeLogEntry, m_timeLog.at(index.row())));
        break;
    case CommentRole:
        m_timeLog[index.row()].comment = value.toString();
        QMetaObject::invokeMethod(m_history, "edit", Qt::QueuedConnection,
                                  Q_ARG(TimeLogEntry, m_timeLog.at(index.row())));
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

    foreach (const TimeLogEntry &entry, removed) {
        QMetaObject::invokeMethod(m_history, "remove", Qt::QueuedConnection, Q_ARG(QUuid, entry.uuid));
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

    QMetaObject::invokeMethod(m_history, "insert", Qt::QueuedConnection, Q_ARG(TimeLogEntry, entry));
}

void TimeLogModel::insertItem(const QModelIndex &index, TimeLogData data)
{
    TimeLogEntry entry(QUuid::createUuid(), data);

    beginInsertRows(index.parent(), index.row(), index.row());
    m_timeLog.insert(index.row(), entry);
    endInsertRows();

    QMetaObject::invokeMethod(m_history, "insert", Qt::QueuedConnection, Q_ARG(TimeLogEntry, entry));
}

void TimeLogModel::processRowsInserted(const QModelIndex &parent, int first, int last)
{
    recalcDuration(parent, first, last);
}

void TimeLogModel::processRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(last)

    if (first != 0) {   // No need to recalculate, if items removed at the beginning
        recalcDuration(parent, first-1, first-1);
    }
}

void TimeLogModel::historyError(const QString &errorText)
{
    emit error(QString("Database error: %1").arg(errorText));
    beginResetModel();
    m_timeLog.clear();
    endResetModel();
}

void TimeLogModel::getMoreHistory()
{
    QDateTime limit = m_timeLog.size() ? m_timeLog.at(0).startTime : QDateTime::currentDateTime();
    QVector<TimeLogEntry> data = m_history->getHistory(defaultPopulateCount, limit);
    if (!data.size()) {
        return;
    }

    beginInsertRows(QModelIndex(), 0, data.size() - 1);
    data.append(m_timeLog);
    m_timeLog.swap(data);
    endInsertRows();
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
        m_timeLog[last].durationTime = m_timeLog.at(last).startTime.secsTo(QDateTime::currentDateTimeUtc());
    }

    emit dataChanged(index(start, 0, parent), index(last, 0, parent),
                     QVector<int>() << DurationTimeRole);
}
