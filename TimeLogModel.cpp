#include <QDebug>

#include "TimeLogModel.h"

TimeLogModel::TimeLogModel(QObject *parent) :
    SUPER(parent)
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(processRowsInserted(QModelIndex,int,int)));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(processRowsRemoved(QModelIndex,int,int)));
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
        break;
    }
    case DurationTimeRole:
        m_timeLog[index.row()].durationTime = value.toInt();
        break;
    case CategoryRole:
        m_timeLog[index.row()].category = value.toString();
        break;
    case CommentRole:
        m_timeLog[index.row()].comment = value.toString();
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, QVector<int>() << role);

    return true;
}

void TimeLogModel::addItem(TimeLogData data)
{
    int itemIndex = m_timeLog.size();
    beginInsertRows(QModelIndex(), itemIndex, itemIndex);
    m_timeLog.append(TimeLogEntry(data));
    endInsertRows();
}

void TimeLogModel::removeItem(const QModelIndex &index)
{
    beginRemoveRows(index.parent(), index.row(), index.row());
    m_timeLog.removeAt(index.row());
    endRemoveRows();
}

void TimeLogModel::processRowsInserted(const QModelIndex &parent, int first, int last)
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

void TimeLogModel::processRowsRemoved(const QModelIndex &parent, int first, int last)
{
    if (first != 0) {   // No need to recalculate, if items removed at the beginning
        if (first == m_timeLog.size())  {   // Items removed at the end of the list
            m_timeLog[first-1].durationTime = m_timeLog.at(first-1).startTime.secsTo(QDateTime::currentDateTimeUtc());
        } else {
            m_timeLog[first-1].durationTime = m_timeLog.at(first-1).startTime.secsTo(m_timeLog.at(first).startTime);
        }

        emit dataChanged(index(first - 1, 0, parent), index(first - 1, 0, parent),
                         QVector<int>() << DurationTimeRole);
    }
}
