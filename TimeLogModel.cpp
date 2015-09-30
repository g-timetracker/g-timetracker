#include "TimeLogModel.h"

Q_LOGGING_CATEGORY(TIME_LOG_MODEL_CATEGORY, "TimeLogModel", QtInfoMsg)

class uuidEquals
{
public:
    uuidEquals(const QUuid &uuid) : m_uuid(uuid) { }

    bool operator ()(const TimeLogEntry &entry) const
    {
        return m_uuid == entry.uuid;
    }

private:
    QUuid m_uuid;
};

TimeLogModel::TimeLogModel(QObject *parent) :
    SUPER(parent),
    m_history(TimeLogHistory::instance())
{
    connect(m_history, SIGNAL(error(QString)),
            this, SLOT(historyError(QString)));
    connect(m_history, SIGNAL(dataAvailable(QVector<TimeLogEntry>,QDateTime)),
            this, SLOT(historyDataAvailable(QVector<TimeLogEntry>,QDateTime)));
    connect(m_history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)),
            this, SLOT(historyDataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
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
        if (m_timeLog[index.row()].durationTime == -1) {
            return QVariant::fromValue(m_timeLog[index.row()].startTime.secsTo(QDateTime::currentDateTime()));
        } else {
            return QVariant::fromValue(m_timeLog[index.row()].durationTime);
        }
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

    foreach (const TimeLogEntry &entry, removed) {
        m_history->remove(entry);
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

    m_history->insert(entry);
}

void TimeLogModel::insertItem(const QModelIndex &index, TimeLogData data)
{
    TimeLogEntry entry(QUuid::createUuid(), data);

    beginInsertRows(index.parent(), index.row(), index.row());
    m_timeLog.insert(index.row(), entry);
    endInsertRows();

    m_history->insert(entry);
}

void TimeLogModel::historyError(const QString &errorText)
{
    Q_UNUSED(errorText)

    clear();
}

void TimeLogModel::historyDataAvailable(QVector<TimeLogEntry> data, QDateTime until)
{
    if (!m_requestedData.contains(until)) {
        qCDebug(TIME_LOG_MODEL_CATEGORY) << "Discarding received but not requested data for time"
                                         << until;
        return;
    }

    processHistoryData(data);

    m_requestedData.remove(until);
}

void TimeLogModel::historyDataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields)
{
    Q_ASSERT(data.size() == fields.size());

    for (int i = 0; i < data.size(); i++) {
        const TimeLogEntry &entry = data.at(i);
        int dataIndex = findData(entry);
        if (dataIndex == -1) {
            continue;
        }

        QVector<int> roles;
        if (fields.at(i) & TimeLogHistory::DurationTime) {
            m_timeLog[dataIndex].durationTime = entry.durationTime;
            roles.append(DurationTimeRole);
        }
        if (fields.at(i) & TimeLogHistory::StartTime) {
            m_timeLog[dataIndex].startTime = entry.startTime;
            roles.append(StartTimeRole);
        }
        if (fields.at(i) & TimeLogHistory::Category) {
            m_timeLog[dataIndex].category = entry.category;
            roles.append(CategoryRole);
        }
        if (fields.at(i) & TimeLogHistory::Comment) {
            m_timeLog[dataIndex].comment = entry.comment;
            roles.append(CommentRole);
        }
        QModelIndex itemIndex = index(dataIndex, 0, QModelIndex());
        emit dataChanged(itemIndex, itemIndex, roles);
    }
}

void TimeLogModel::clear()
{
    beginResetModel();
    m_timeLog.clear();
    m_requestedData.clear();
    endResetModel();
}

void TimeLogModel::processHistoryData(QVector<TimeLogEntry> data)
{
    if (!data.size()) {
        return;
    }

    int index = m_timeLog.size();

    beginInsertRows(QModelIndex(), index, index + data.size() - 1);
    m_timeLog.append(data);
    endInsertRows();
}

int TimeLogModel::findData(const TimeLogEntry &entry) const
{
    QVector<TimeLogEntry>::const_iterator it = std::find_if(m_timeLog.begin(), m_timeLog.end(),
                                                            uuidEquals(entry.uuid));
    if (it == m_timeLog.end()) {
        qCWarning(TIME_LOG_MODEL_CATEGORY) << "Item not found:\n"
                                           << entry.startTime << entry.category << entry.uuid;
        return -1;
    } else {
        return (it - m_timeLog.begin());
    }
}
