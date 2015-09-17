#ifndef DATAIMPORTER_H
#define DATAIMPORTER_H

#include "AbstractDataInOut.h"
#include "TimeLogEntry.h"

class DataImporter: public AbstractDataInOut
{
    Q_OBJECT
public:
    explicit DataImporter(QObject *parent = 0);

protected slots:
    virtual void startIO(const QString &path);
    virtual void historyError(const QString &errorText);

private slots:
    void historyDataInserted(QVector<TimeLogEntry> data);

private:
    bool processPath(const QString &path);
    bool processDirectory(const QString &path);
    void importCurrentFile();
    void importFile(const QString &path);
    QVector<TimeLogEntry> parseFile(const QString &path) const;
    TimeLogEntry parseLine(const QString &line) const;

    QStringList m_fileList;
    int m_currentIndex;
};

#endif // DATAIMPORTER_H
