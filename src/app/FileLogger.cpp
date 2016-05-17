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

#include "FileLogger.h"

#include <QLoggingCategory>
#include <QStandardPaths>
#include <QDir>
#include <QMutex>
#include <QDateTime>
#include <QCoreApplication>

static QFile *logFile(Q_NULLPTR);
static QMutex streamMutex;
static QtMessageHandler defaultHandler;

void FileLogger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QMutexLocker locker(&streamMutex);

    static QTextStream logStream(logFile);

    logStream << qFormatLogMessage(type, context, message) << endl;

    defaultHandler(type, context, message);
}

void FileLogger::setup()
{
    QMutexLocker locker(&streamMutex);

    QString path(QString("%1/logs").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));
    QDir().mkpath(path);
    QString name = QString("%1/debug.log").arg(path);
    logFile = new QFile(name, QCoreApplication::instance());
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qCritical() << "Fail to open log file:" << logFile->errorString() << endl;
        delete logFile;
        return;
    }

    defaultHandler = qInstallMessageHandler(FileLogger::messageHandler);
}
