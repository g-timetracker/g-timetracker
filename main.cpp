#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QtQml/QQmlContext>

#include <QLoggingCategory>

#include "TimeLogHistory.h"
#include "TimeLogRecentModel.h"
#include "TimeLogSearchModel.h"
#include "ReverseProxyModel.h"
#include "TimeLog.h"
#include "DataImporter.h"

Q_LOGGING_CATEGORY(MAIN_CATEGORY, "main", QtInfoMsg)

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("G-TimeTracker");
    app.setOrganizationDomain("g-timetracker.org");
    app.setApplicationName("G-TimeTracker");
    app.setApplicationVersion("0.1");

    qSetMessagePattern("[%{time}] <%{category}> %{type} (%{file}:%{line}, %{function}) %{message}");

    QCommandLineParser parser;
    parser.setApplicationDescription("Global Time Tracker");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption importOption(QStringList() << "i" << "import", "Import a CSV file(s)",
                                    "file or directory");
    parser.addOption(importOption);
    QCommandLineOption separatorOption("separator", "Separator for import", "string", ";");
    parser.addOption(separatorOption);

    parser.process(app);

    if (!TimeLogHistory::instance()->init()) {
        qCCritical(MAIN_CATEGORY) << "Fail to initialize db";
        return EXIT_FAILURE;
    }

    qRegisterMetaType<TimeLogData>();
    qRegisterMetaType<TimeLogEntry>();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();

    TimeLogHistory::instance()->madeAsync();

    if (parser.isSet(importOption)) {
        DataImporter importer;
        importer.setSeparator(parser.value(separatorOption));
        importer.import(parser.value(importOption));
        return app.exec();
    } else {
        qmlRegisterType<TimeLogModel>("TimeLog", 1, 0, "TimeLogModel");
        qmlRegisterType<TimeLogRecentModel>("TimeLog", 1, 0, "TimeLogRecentModel");
        qmlRegisterType<TimeLogSearchModel>("TimeLog", 1, 0, "TimeLogSearchModel");
        qmlRegisterType<ReverseProxyModel>("TimeLog", 1, 0, "ReverseProxyModel");
        qmlRegisterSingletonType(QUrl("qrc:/Settings.qml"), "TimeLog", 1, 0, "Settings");

        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty("TimeLog", TimeLog::instance());
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        return app.exec();
    }
}
