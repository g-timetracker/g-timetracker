#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include <QLoggingCategory>

#include "DataExporter.h"
#include "TimeLogHistory.h"

Q_LOGGING_CATEGORY(DATA_EXPORTER_CATEGORY, "DataExporter", QtInfoMsg)

DataExporter::DataExporter(QObject *parent) :
    QObject(parent),
    m_db(TimeLogHistory::instance()),
    m_sep(";")
{
    connect(m_db, SIGNAL(error(QString)),
            this, SLOT(historyError(QString)));
    connect(m_db, SIGNAL(dataAvailable(QDateTime,QVector<TimeLogEntry>)),
            this, SLOT(historyDataAvailable(QDateTime,QVector<TimeLogEntry>)));
    connect(m_db, SIGNAL(dataAvailable(QVector<TimeLogEntry>,QDateTime)),
            this, SLOT(historyDataAvailable(QVector<TimeLogEntry>,QDateTime)));
}

void DataExporter::exportData(const QString &path)
{
    QMetaObject::invokeMethod(this, "startExport", Qt::QueuedConnection, Q_ARG(QString, path));
}

void DataExporter::setSeparator(const QString &sep)
{
    m_sep = sep;
}

void DataExporter::startExport(const QString &path)
{
    if (!prepareDir(path)) {
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    if (!m_db->size()) {
        qCInfo(DATA_EXPORTER_CATEGORY) << "No data to export";
        QCoreApplication::quit();
        return;
    }

    m_db->getHistoryAfter(1, QDateTime::fromTime_t(0));
}

void DataExporter::historyError(const QString &errorText)
{
    qCCritical(DATA_EXPORTER_CATEGORY) << "Fail to get data from db:" << errorText;
    QCoreApplication::exit(EXIT_FAILURE);
}

void DataExporter::historyDataAvailable(QDateTime from, QVector<TimeLogEntry> data)
{
    Q_ASSERT(m_currentDate.isNull());
    Q_UNUSED(from)

    m_currentDate = data.first().startTime.date();
    m_db->getHistoryBefore(1, QDateTime::currentDateTime());
}

void DataExporter::historyDataAvailable(QVector<TimeLogEntry> data, QDateTime until)
{
    Q_ASSERT(m_currentDate.isValid());
    Q_UNUSED(until)

    if (m_currentDate == m_lastDate) {
        qCInfo(DATA_EXPORTER_CATEGORY) << "All data successfully exported";
        QCoreApplication::quit();
        return;
    } else if (m_lastDate.isNull()) {
        m_lastDate = data.last().startTime.date();
    } else {
        if (!exportDay(data)) {
            QCoreApplication::exit(EXIT_FAILURE);
            return;
        }
        m_currentDate = m_currentDate.addDays(1);
    }

    exportCurrentDate();
}

bool DataExporter::prepareDir(const QString &path)
{
    m_dir.setPath(path);
    if (!m_dir.exists()) {
        qCDebug(DATA_EXPORTER_CATEGORY) << QString("Path %1 does not exists, creating").arg(path);
        if (!m_dir.mkpath(path)) {
            qCCritical(DATA_EXPORTER_CATEGORY) << "Fail create destination directory";
            return false;
        }
    }

    QStringList entries = m_dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    if (!entries.isEmpty()) {
        qCCritical(DATA_EXPORTER_CATEGORY) << QString("Target directory is not empty");
        return false;
    }

    return true;
}

void DataExporter::exportCurrentDate()
{
    m_db->getHistoryBetween(QDateTime(m_currentDate), QDateTime(m_currentDate.addDays(1)).addSecs(-1));
}

bool DataExporter::exportDay(QVector<TimeLogEntry> data)
{
    if (data.isEmpty()) {
        qCInfo(DATA_EXPORTER_CATEGORY) << QString("No data for date %1").arg(m_currentDate.toString());
        return true;
    }

    qCInfo(DATA_EXPORTER_CATEGORY) << QString("Exporting data for date %1").arg(m_currentDate.toString());

    QString fileName = QString("%1 (%2).csv").arg(m_currentDate.toString(Qt::ISODate))
                                             .arg(m_currentDate.toString("ddd"));
    QString filePath = m_dir.filePath(fileName);
    QFile file(filePath);
    if (file.exists()) {
        qCCritical(DATA_EXPORTER_CATEGORY) << "File already exists" << filePath;
        return false;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCCritical(DATA_EXPORTER_CATEGORY) << "Fail to open file" << filePath << file.errorString();
        return false;
    }

    QStringList strings;
    foreach (const TimeLogEntry &entry, data) {
        QStringList values;
        values << entry.startTime.toString(Qt::ISODate);
        values << entry.category;
        values << entry.comment;
        values << entry.uuid.toString();
        strings.append(values.join(m_sep));
    }

    QTextStream stream(&file);
    stream << strings.join('\n') << endl;
    if (stream.status() != QTextStream::Ok || file.error() != QFileDevice::NoError) {
        qCCritical(DATA_EXPORTER_CATEGORY) << "Error writing to file" << file.errorString();
        return false;
    }

    return true;
}
