#include <QDebug>

#include "TimeLogModel.h"
#include "TimeLogHistory.h"

static const int defaultPopulateCount(5);

TimeLogModel::TimeLogModel(QObject *parent) :
    SUPER(parent),
    m_history(new TimeLogHistory(this))
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(processRowsInserted(QModelIndex,int,int)), Qt::QueuedConnection);
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(processRowsRemoved(QModelIndex,int,int)), Qt::QueuedConnection);

    populate();
}

int TimeLogModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_timeLog.size();
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
        m_history->edit(m_timeLog.at(index.row()));
        recalcDuration(index.parent(), index.row(), index.row());
        break;
    }
    case DurationTimeRole:
        return false;   // This property can only be calculated
    case CategoryRole:
        m_timeLog[index.row()].category = value.toString();
        m_history->edit(m_timeLog.at(index.row()));
        break;
    case CommentRole:
        m_timeLog[index.row()].comment = value.toString();
        m_history->edit(m_timeLog.at(index.row()));
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

    beginRemoveRows(parent, row, row + count - 1);

    for (int i = row; i < row + count; i++) {
        m_history->remove(m_timeLog.at(i).uuid);
    }

    m_timeLog.remove(row, count);

    endRemoveRows();

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
    int itemIndex = m_timeLog.size();
    beginInsertRows(QModelIndex(), itemIndex, itemIndex);
    TimeLogEntry entry(QUuid::createUuid(), data);
    m_timeLog.append(entry);
    m_history->insert(entry);
    endInsertRows();
}

void TimeLogModel::insertItem(const QModelIndex &index, TimeLogData data)
{
    beginInsertRows(index.parent(), index.row(), index.row());
    TimeLogEntry entry(QUuid::createUuid(), data);
    m_timeLog.insert(index.row(), entry);
    m_history->insert(entry);
    endInsertRows();
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

void TimeLogModel::populate()
{
    QVector<TimeLogEntry> data = m_history->getHistory(defaultPopulateCount);
    if (!data.size()) {
        return;
    }

    beginInsertRows(QModelIndex(), 0, data.size() - 1);
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
