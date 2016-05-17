/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef TIMELOGSEARCHMODEL_H
#define TIMELOGSEARCHMODEL_H

#include "TimeLogModel.h"

class TimeLogSearchModel : public TimeLogModel
{
    Q_OBJECT
    Q_PROPERTY(QDateTime begin MEMBER m_begin NOTIFY beginChanged)
    Q_PROPERTY(QDateTime end MEMBER m_end NOTIFY endChanged)
    Q_PROPERTY(QString category MEMBER m_category NOTIFY categoryChanged)
    Q_PROPERTY(bool withSubcategories MEMBER m_withSubcategories NOTIFY withSubcategoruesChanged)
    typedef TimeLogModel SUPER;
public:
    explicit TimeLogSearchModel(QObject *parent = 0);

signals:
    void beginChanged(const QDateTime &begin);
    void endChanged(const QDateTime &end);
    void categoryChanged(const QString &category);
    void withSubcategoruesChanged(bool withSubcategories);

private slots:
    void updateData();

private:
    virtual void processDataInsert(TimeLogEntry data);
    virtual int findData(const TimeLogEntry &entry) const;

    QDateTime m_begin;
    QDateTime m_end;
    QString m_category;
    bool m_withSubcategories;
};

#endif // TIMELOGSEARCHMODEL_H
