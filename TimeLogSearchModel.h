#ifndef TIMELOGSEARCHMODEL_H
#define TIMELOGSEARCHMODEL_H

#include "TimeLogModel.h"

class TimeLogSearchModel : public TimeLogModel
{
    Q_OBJECT
    Q_PROPERTY(QDateTime begin MEMBER m_begin NOTIFY beginChanged)
    Q_PROPERTY(QDateTime end MEMBER m_end NOTIFY endChanged)
    Q_PROPERTY(QString category MEMBER m_category NOTIFY categoryChanged)
    typedef TimeLogModel SUPER;
public:
    explicit TimeLogSearchModel(QObject *parent = 0);

signals:
    void beginChanged(const QDateTime &begin);
    void endChanged(const QDateTime &end);
    void categoryChanged(const QString &category);

private slots:
    void updateData();

private:
    virtual void processDataInsert(QVector<TimeLogEntry> data);
    virtual int findData(const TimeLogEntry &entry) const;

    QDateTime m_begin;
    QDateTime m_end;
    QString m_category;
};

#endif // TIMELOGSEARCHMODEL_H
