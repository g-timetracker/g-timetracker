#ifndef TIMELOGRECENTMODEL_H
#define TIMELOGRECENTMODEL_H

#include "TimeLogModel.h"

class TimeLogRecentModel : public TimeLogModel
{
    Q_OBJECT
    typedef TimeLogModel SUPER;
public:
    explicit TimeLogRecentModel(QObject *parent = 0);

    virtual bool canFetchMore(const QModelIndex &parent) const;
    virtual void fetchMore(const QModelIndex & parent);

protected:
    virtual void processHistoryData(QVector<TimeLogEntry> data);
    virtual void processDataInsert(TimeLogEntry data);
    virtual int findData(const TimeLogEntry &entry) const;

private:
    void getMoreHistory();
};

#endif // TIMELOGRECENTMODEL_H
