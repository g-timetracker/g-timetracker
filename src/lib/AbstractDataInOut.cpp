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

#include <QCoreApplication>

#include "AbstractDataInOut.h"
#include "TimeTracker.h"
#include "TimeLogHistory.h"

Q_LOGGING_CATEGORY(DATA_IO_CATEGORY, "DataIO", QtInfoMsg)

AbstractDataInOut::AbstractDataInOut(TimeLogHistory *db, QObject *parent) :
    QObject(parent),
    m_db(db),
    m_sep(";")
{
    connect(m_db, SIGNAL(error(QString)),
            this, SLOT(historyError(QString)));
}

void AbstractDataInOut::setSeparator(const QString &sep)
{
    m_sep = sep;
}

void AbstractDataInOut::start(const QString &path)
{
    QMetaObject::invokeMethod(this, "startIO", Qt::QueuedConnection, Q_ARG(QString, path));
}

QString AbstractDataInOut::formatFileError(const QString &message, const QFile &file)
{
    return QString("%1 %2: %3 (code %4)").arg(message).arg(file.fileName())
                                         .arg(file.errorString()).arg(file.error());
}

QStringList AbstractDataInOut::buildFileList(const QString &path, bool isRecursive, QStringList filters)
{
    QStringList result;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        qCWarning(DATA_IO_CATEGORY) << QString("Path %1 does not exists").arg(path);
        return result;
    }

    if (fileInfo.isFile()) {
        result.append(path);
    } else if (fileInfo.isDir()) {
        QDir dir(path);
        QStringList entries;
        entries = dir.entryList(filters, QDir::Files);
        foreach (const QString &entry, entries) {
            result.append(dir.filePath(entry));
        }
        if (isRecursive) {
            entries = dir.entryList(filters, QDir::Dirs | QDir::NoDotAndDotDot);
            foreach (const QString &entry, entries) {
                result.append(buildFileList(dir.filePath(entry), isRecursive, filters));
            }
        }
    }

    return result;
}

bool AbstractDataInOut::prepareDir(const QString &path, QDir &dir)
{
    dir.setPath(path);
    if (!dir.exists()) {
        qCDebug(DATA_IO_CATEGORY) << QString("Path %1 does not exists, creating").arg(path);
        if (!dir.mkpath(path)) {
            return false;
        }
    }

    return true;
}
