#ifndef ABSTRACTDATAINOUT_H
#define ABSTRACTDATAINOUT_H

#include <QObject>
#include <QFile>
#include <QDir>

#include <QLoggingCategory>

class TimeLogHistory;

class AbstractDataInOut : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDataInOut(QObject *parent = 0);

    void setSeparator(const QString &sep);
    void start(const QString &path);

    static QString formatFileError(const QString &message, const QFile &file);

protected slots:
    virtual void startIO(const QString &path) = 0;
    virtual void historyError(const QString &errorText) = 0;

protected:
    QStringList buildFileList(const QString &path) const;
    bool prepareDir(const QString &path, QDir &dir) const;

    TimeLogHistory *m_db;
    QString m_sep;
};

Q_DECLARE_LOGGING_CATEGORY(DATA_IO_CATEGORY)

#endif // ABSTRACTDATAINOUT_H
