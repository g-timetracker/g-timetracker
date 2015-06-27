#include <QDebug>

#include "TimeLogModel.h"

TimeLogModel::TimeLogModel(QObject *parent) :
    SUPER(parent)
{
}

void TimeLogModel::addItem()
{
    qDebug() << "addItem";
    int itemIndex = m_timeLog.size();
    beginInsertRows(QModelIndex(), itemIndex, itemIndex);
    m_timeLog.append(TimeLogEntry());
    endInsertRows();
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
        m_timeLog[index.row()].durationTime = value.toLongLong();
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

    emit dataChanged(index, index);

    return true;
}

bool TimeLogModel::insertRows(int row, int count, const QModelIndex &parent)
{

}

bool TimeLogModel::removeRows(int row, int count, const QModelIndex &parent)
{

}
