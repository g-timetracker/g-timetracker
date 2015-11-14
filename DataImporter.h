#ifndef DATAIMPORTER_H
#define DATAIMPORTER_H

#include "AbstractDataInOut.h"
#include "TimeLogEntry.h"

class DataImporter: public AbstractDataInOut
{
    Q_OBJECT
public:
    explicit DataImporter(TimeLogHistory *db, QObject *parent = 0);

protected slots:
    virtual void startIO(const QString &path);
    virtual void historyError(const QString &errorText);

private slots:
    void historyDataImported(QVector<TimeLogEntry> data);

private:
    void importCurrentFile();
    void importFile(const QString &path);
    QVector<TimeLogEntry> parseFile(const QString &path) const;
    TimeLogEntry parseLine(const QString &line) const;

    QStringList m_fileList;
    int m_currentIndex;
};

#endif // DATAIMPORTER_H
