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

    QString path(QString("%1/logs").arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)));
    QDir().mkpath(path);
    QString name = QString("%1/%2.log").arg(path).arg(QDateTime::currentDateTime().toString(Qt::ISODate));
    logFile = new QFile(name, QCoreApplication::instance());
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qCritical() << "Fail to open log file:" << logFile->errorString() << endl;
        delete logFile;
        return;
    }

    defaultHandler = qInstallMessageHandler(FileLogger::messageHandler);
}
