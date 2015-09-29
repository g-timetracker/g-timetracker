#include <QCoreApplication>

#include "AbstractDataInOut.h"
#include "TimeLogHistory.h"

Q_LOGGING_CATEGORY(DATA_IO_CATEGORY, "DataIO", QtInfoMsg)

AbstractDataInOut::AbstractDataInOut(QObject *parent) :
    QObject(parent),
    m_db(TimeLogHistory::instance()),
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

bool AbstractDataInOut::buildFileList(const QString &path, QStringList &fileList)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        qCCritical(DATA_IO_CATEGORY) << "Path does not exists" << path;;
        return false;
    }

    if (fileInfo.isFile()) {
        fileList.append(path);
    } else if (fileInfo.isDir()) {
        QDir dir(path);
        QStringList entries;
        entries = dir.entryList(QDir::Files);
        foreach (const QString &entry, entries) {
            fileList.append(dir.filePath(entry));
        }
        entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (const QString &entry, entries) {
            if (!buildFileList(dir.filePath(entry), fileList)) {
                return false;
            }
        }
    } else {
        qCCritical(DATA_IO_CATEGORY) << "Not file or directory" << path;
        return false;
    }

    return true;
}

bool AbstractDataInOut::prepareDir(const QString &path, QDir &dir)
{
    dir.setPath(path);
    if (!dir.exists()) {
        qCDebug(DATA_IO_CATEGORY) << QString("Path %1 does not exists, creating").arg(path);
        if (!dir.mkpath(path)) {
            qCCritical(DATA_IO_CATEGORY) << "Fail create destination directory";
            return false;
        }
    }

    return true;
}
