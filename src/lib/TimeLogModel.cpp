#include "TimeLogModel.h"
#include "TimeTracker.h"

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
    m_timeTracker(Q_NULLPTR),
    m_history(Q_NULLPTR)
{

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
    case PrecedingStartRole:
        return QVariant::fromValue(m_timeLog[index.row()].precedingStart);
    case SucceedingStartRole:
        if (m_timeLog[index.row()].durationTime == -1) {
            return QVariant::fromValue(QDateTime::currentDateTimeUtc());
        } else {
            return QVariant::fromValue(QDateTime(m_timeLog[index.row()].startTime).addSecs(m_timeLog[index.row()].durationTime));
        }
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
    roles[PrecedingStartRole] = "precedingStart";
    roles[SucceedingStartRole] = "succeedingStart";

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
    if (!m_history) {
        return false;
    }

    if (!index.isValid()) {
        return false;
    }

    switch (role) {
    case StartTimeRole: {
        QDateTime time = value.toDateTime();
        if (!time.isValid()) {
            return false;
        }
        if (!checkStartValid(index.row() - 1, index.row() + 1, time)) {
            return false;
        }
        m_timeLog[index.row()].startTime = time;
        m_history->edit(m_timeLog.at(index.row()), TimeLogHistory::StartTime);
        break;
    }
    case DurationTimeRole:
    case PrecedingStartRole:
    case SucceedingStartRole:
        return false;   // This properties can only be calculated
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
    if (!m_history) {
        return false;
    }

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

void TimeLogModel::removeItem(const QModelIndex &index)
{
    removeRow(index.row(), index.parent());
}

void TimeLogModel::appendItem(TimeLogData data)
{
    if (!m_history) {
        return;
    }

    if (!checkStartValid(m_timeLog.size() - 1, m_timeLog.size(), data.startTime)) {
        return;
    }

    TimeLogEntry entry(QUuid::createUuid(), data);

    int itemIndex = m_timeLog.size();
    beginInsertRows(QModelIndex(), itemIndex, itemIndex);
    m_timeLog.append(entry);
    endInsertRows();

    m_history->insert(entry);
}

void TimeLogModel::insertItem(const QModelIndex &index, TimeLogData data)
{
    if (!m_history) {
        return;
    }

    if (!checkStartValid(index.row() - 1, index.row(), data.startTime)) {
        return;
    }

    TimeLogEntry entry(QUuid::createUuid(), data);

    beginInsertRows(index.parent(), index.row(), index.row());
    m_timeLog.insert(index.row(), entry);
    endInsertRows();

    m_history->insert(entry);
}

void TimeLogModel::setTimeTracker(TimeTracker *timeTracker)
{
    if (m_timeTracker == timeTracker) {
        return;
    }

    if (m_timeTracker) {
        disconnect(this, SIGNAL(error(QString)),
                   m_timeTracker, SIGNAL(error(QString)));
        disconnect(m_timeTracker, SIGNAL(historyChanged(TimeLogHistory*)),
                   this, SLOT(setHistory(TimeLogHistory*)));
    }

    m_timeTracker = timeTracker;

    if (m_timeTracker) {
        connect(this, SIGNAL(error(QString)),
                m_timeTracker, SIGNAL(error(QString)));
        connect(m_timeTracker, SIGNAL(historyChanged(TimeLogHistory*)),
                this, SLOT(setHistory(TimeLogHistory*)));
    }

    setHistory(m_timeTracker ? m_timeTracker->history() : Q_NULLPTR);

    emit timeTrackerChanged(m_timeTracker);
}

void TimeLogModel::setHistory(TimeLogHistory *history)
{
    if (m_history == history) {
        return;
    }

    if (m_history) {
        disconnect(m_history, SIGNAL(dataOutdated()),
                   this, SLOT(historyDataOutdated()));
        disconnect(m_history, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)),
                   this, SLOT(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));
        disconnect(m_history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)),
                   this, SLOT(historyDataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
        disconnect(m_history, SIGNAL(dataInserted(TimeLogEntry)),
                   this, SLOT(historyDataInserted(TimeLogEntry)));
        disconnect(m_history, SIGNAL(dataRemoved(TimeLogEntry)),
                   this, SLOT(historyDataRemoved(TimeLogEntry)));
    }

    m_history = history;

    if (history) {
        connect(m_history, SIGNAL(dataOutdated()),
                this, SLOT(historyDataOutdated()));
        connect(m_history, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)),
                this, SLOT(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));
        connect(m_history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)),
                this, SLOT(historyDataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
        connect(m_history, SIGNAL(dataInserted(TimeLogEntry)),
                this, SLOT(historyDataInserted(TimeLogEntry)));
        connect(m_history, SIGNAL(dataRemoved(TimeLogEntry)),
                this, SLOT(historyDataRemoved(TimeLogEntry)));
    }

    m_pendingRequests.clear();
    m_obsoleteRequests.clear();

    clear();
}

void TimeLogModel::historyDataOutdated()
{
    clear();
}

void TimeLogModel::historyRequestCompleted(QVector<TimeLogEntry> data, qlonglong id)
{
    if (m_obsoleteRequests.removeOne(id)) {
        return;
    }
    if (!m_pendingRequests.removeOne(id)) {
        qCDebug(TIME_LOG_MODEL_CATEGORY) << "Discarding received but not requested data for id" << id;
        return;
    }

    processHistoryData(data);
}

void TimeLogModel::historyDataUpdated(QVector<TimeLogEntry> data, QVector<TimeLogHistory::Fields> fields)
{
    Q_ASSERT(data.size() == fields.size());

    int dataIndex = -1;

    for (int i = 0; i < data.size(); i++) {
        const TimeLogEntry &entry = data.at(i);

        if (dataIndex != -1 && dataIndex < m_timeLog.size() - 1
            && entry.uuid == m_timeLog.at(dataIndex+1).uuid) {
            // If current item to update follows right after the previous, no search needed
            dataIndex++;
        } else {
            // Serch only by UUID if start time changed
            dataIndex = (fields.at(i) & TimeLogHistory::StartTime) ? TimeLogModel::findData(entry)
                                                                   : findData(entry);
        }
        if (dataIndex == -1) {
            continue;
        }

        QVector<int> roles;
        if (fields.at(i) & TimeLogHistory::DurationTime) {
            m_timeLog[dataIndex].durationTime = entry.durationTime;
            roles.append(DurationTimeRole);
            roles.append(SucceedingStartRole);
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
        if (fields.at(i) & TimeLogHistory::PrecedingStart) {
            m_timeLog[dataIndex].precedingStart = entry.precedingStart;
            roles.append(PrecedingStartRole);
        }
        QModelIndex itemIndex = index(dataIndex, 0, QModelIndex());
        emit dataChanged(itemIndex, itemIndex, roles);
    }
}

void TimeLogModel::historyDataInserted(TimeLogEntry data)
{
    processDataInsert(data);
}

void TimeLogModel::historyDataRemoved(TimeLogEntry data)
{
    processDataRemove(data);
}

void TimeLogModel::clear()
{
    beginResetModel();
    m_timeLog.clear();
    m_obsoleteRequests.append(m_pendingRequests);
    m_pendingRequests.clear();
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

void TimeLogModel::processDataInsert(TimeLogEntry data)
{
    Q_UNUSED(data)
}

void TimeLogModel::processDataRemove(const TimeLogEntry &data)
{
    int index = findData(data);
    if (index == -1) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_timeLog.remove(index, 1);
    endRemoveRows();
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

bool TimeLogModel::checkStartValid(int indexBefore, int indexAfter, const QDateTime &startTime)
{
    Q_ASSERT(indexBefore < indexAfter);
    Q_ASSERT(indexAfter <= m_timeLog.size());
    Q_ASSERT(startTime.isValid());

    if ((indexBefore == -1 || m_timeLog.at(indexBefore).startTime < startTime)
        && (indexAfter == m_timeLog.size() || m_timeLog.at(indexAfter).startTime > startTime)) {
        return true;
    } else {
        emit error(QString("Time %1 doesn't fall within a proper range").arg(startTime.toString(Qt::ISODate)));
        return false;
    }
}

bool TimeLogModel::startTimeCompare(const TimeLogEntry &a, const TimeLogEntry &b)
{
    return a.startTime < b.startTime;
}
