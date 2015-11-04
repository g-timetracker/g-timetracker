#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include "DataExporter.h"
#include "TimeLogHistory.h"


#define fail(message) \
    do {    \
        qCCritical(DATA_IO_CATEGORY) << message;    \
        QCoreApplication::exit(EXIT_FAILURE);   \
    } while (0)

DataExporter::DataExporter(QObject *parent) :
    AbstractDataInOut(parent)
{
    connect(m_db, SIGNAL(historyDataAvailable(QDateTime,QVector<TimeLogEntry>)),
            this, SLOT(historyDataAvailable(QDateTime,QVector<TimeLogEntry>)));
    connect(m_db, SIGNAL(historyDataAvailable(QVector<TimeLogEntry>,QDateTime)),
            this, SLOT(historyDataAvailable(QVector<TimeLogEntry>,QDateTime)));
}

void DataExporter::startIO(const QString &path)
{
    if (!prepareDir(path, m_dir)) {
        fail(QString("Fail to prepare directory %1").arg(path));
        return;
    } else if (!m_dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty()) {
        fail(QString("Target directory %1 is not empty").arg(m_dir.path()));
        return;
    }

    if (!m_db->size()) {
        qCInfo(DATA_IO_CATEGORY) << "No data to export";
        QCoreApplication::quit();
        return;
    }

    m_db->getHistoryAfter(1, QDateTime::fromTime_t(0));
}

void DataExporter::historyError(const QString &errorText)
{
    fail(QString("Fail to get data from db: %1").arg(errorText));
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
        qCInfo(DATA_IO_CATEGORY) << "All data successfully exported";
        QCoreApplication::quit();
        return;
    } else if (m_lastDate.isNull()) {
        m_lastDate = data.last().startTime.date();
    } else {
        if (!exportDay(data)) {
            return;
        }
        m_currentDate = m_currentDate.addDays(1);
    }

    exportCurrentDate();
}

void DataExporter::exportCurrentDate()
{
    m_db->getHistoryBetween(QDateTime(m_currentDate), QDateTime(m_currentDate.addDays(1)).addSecs(-1));
}

bool DataExporter::exportDay(QVector<TimeLogEntry> data)
{
    if (data.isEmpty()) {
        qCInfo(DATA_IO_CATEGORY) << QString("No data for date %1").arg(m_currentDate.toString());
        return true;
    }

    qCInfo(DATA_IO_CATEGORY) << QString("Exporting data for date %1").arg(m_currentDate.toString());

    QString fileName = QString("%1 (%2).csv").arg(m_currentDate.toString(Qt::ISODate))
                                             .arg(m_currentDate.toString("ddd"));
    QString filePath = m_dir.filePath(fileName);
    QFile file(filePath);
    if (file.exists()) {
        fail(QString("File %1 already exists").arg(filePath));
        return false;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        fail(formatFileError("Fail to open file", file));
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
        fail(formatFileError("Error writing to file", file));
        return false;
    }

    return true;
}
