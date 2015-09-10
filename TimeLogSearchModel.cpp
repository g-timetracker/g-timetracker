#include "TimeLogSearchModel.h"

TimeLogSearchModel::TimeLogSearchModel(QObject *parent) :
    SUPER(parent),
    m_begin(QDateTime::fromTime_t(0)),
    m_end(QDateTime::fromTime_t(0))
{
    connect(this, SIGNAL(beginChanged(QDateTime)),
            this, SLOT(updateData()));
    connect(this, SIGNAL(endChanged(QDateTime)),
            this, SLOT(updateData()));
    connect(this, SIGNAL(categoryChanged(QString)),
            this, SLOT(updateData()));
}

void TimeLogSearchModel::updateData()
{
    clear();
    m_requestedData.insert(m_end);
    m_history->getHistory(m_begin, m_end, m_category);
}
