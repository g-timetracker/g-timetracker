#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QtGlobal>

class FileLogger
{
public:
    static void setup();

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext &context,
                               const QString &message);
};

#endif // FILELOGGER_H
