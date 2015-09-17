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

void AbstractDataInOut::historyError(const QString &errorText)
{
    qCCritical(DATA_IO_CATEGORY) << "Fail to get data from db:" << errorText;
    QCoreApplication::exit(EXIT_FAILURE);
}
