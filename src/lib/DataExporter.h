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

#ifndef DATAEXPORTER_H
#define DATAEXPORTER_H

#include "AbstractDataInOut.h"
#include "TimeLogEntry.h"
#include "TimeLogCategory.h"

class DataExporter : public AbstractDataInOut
{
    Q_OBJECT
public:
    explicit DataExporter(TimeLogHistory *db, QObject *parent = 0);

protected slots:
    virtual void startIO(const QString &path);
    virtual void historyError(const QString &errorText);

private slots:
    void historyRequestCompleted(QVector<TimeLogEntry> data, qlonglong id);
    void storedCategoriesAvailable(QVector<TimeLogCategory> data);

private:
    void exportCurrentDate(qlonglong id);
    bool exportDay(const QVector<TimeLogEntry> &data);
    bool exportCategories(const QVector<TimeLogCategory> &data);

    QDir m_dir;
    QDate m_currentDate;
    QDate m_lastDate;
};

#endif // DATAEXPORTER_H
