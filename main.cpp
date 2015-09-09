#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include <QLoggingCategory>

#include "TimeLogHistory.h"
#include "TimeLogModel.h"
#include "ReverseProxyModel.h"
#include "TimeLogSingleton.h"
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

    if (parser.isSet(importOption)) {
        DataImporter importer;
        importer.setSeparator(parser.value(separatorOption));
        return (importer.import(parser.value(importOption)) ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    qRegisterMetaType<TimeLogEntry>();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();

    TimeLogHistory::instance()->madeAsync();
    TimeLogSingleton singleton;
    qRegisterMetaType<TimeLogData>();
    TimeLogModel model;
    QObject::connect(&model, SIGNAL(error(QString)), &singleton, SIGNAL(error(QString)));
    ReverseProxyModel proxy;
    proxy.setSourceModel(&model);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("TimeLogModel", &proxy);
    engine.rootContext()->setContextProperty("TimeLog", &singleton);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
