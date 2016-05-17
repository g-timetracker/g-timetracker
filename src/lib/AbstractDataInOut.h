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

#ifndef ABSTRACTDATAINOUT_H
#define ABSTRACTDATAINOUT_H

#include <QObject>
#include <QFile>
#include <QDir>

#include <QLoggingCategory>

class TimeLogHistory;

class AbstractDataInOut : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDataInOut(TimeLogHistory *db, QObject *parent = 0);

    void setSeparator(const QString &sep);
    Q_INVOKABLE void start(const QString &path);

    static QString formatFileError(const QString &message, const QFile &file);
    static QStringList buildFileList(const QString &path, bool isRecursive = false,
                                     QStringList filters = QStringList());
    static bool prepareDir(const QString &path, QDir &dir);

protected slots:
    virtual void startIO(const QString &path) = 0;
    virtual void historyError(const QString &errorText) = 0;

protected:
    TimeLogHistory *m_db;
    QString m_sep;
};

Q_DECLARE_LOGGING_CATEGORY(DATA_IO_CATEGORY)

#endif // ABSTRACTDATAINOUT_H
