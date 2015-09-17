#ifndef ABSTRACTDATAINOUT_H
#define ABSTRACTDATAINOUT_H

#include <QObject>

#include <QLoggingCategory>

class TimeLogHistory;

class AbstractDataInOut : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDataInOut(QObject *parent = 0);

    void setSeparator(const QString &sep);
    void start(const QString &path);

protected slots:
    virtual void startIO(const QString &path) = 0;
    virtual void historyError(const QString &errorText);

protected:
    TimeLogHistory *m_db;
    QString m_sep;
};

Q_DECLARE_LOGGING_CATEGORY(DATA_IO_CATEGORY)

#endif // ABSTRACTDATAINOUT_H
