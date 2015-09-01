#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include "TimeLogModel.h"
#include "ReverseProxyModel.h"
#include "TimeLogSingleton.h"
#include "DataImporter.h"

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

    if (parser.isSet(importOption)) {
        DataImporter importer;
        importer.setSeparator(parser.value(separatorOption));
        return (importer.import(parser.value(importOption)) ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    TimeLogSingleton *singleton = new TimeLogSingleton;
    qRegisterMetaType<TimeLogData>();
    TimeLogModel *model = new TimeLogModel;
    QObject::connect(model, SIGNAL(error(QString)), singleton, SIGNAL(error(QString)));
    ReverseProxyModel *proxy = new ReverseProxyModel;
    proxy->setSourceModel(model);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("TimeLogModel", proxy);
    engine.rootContext()->setContextProperty("TimeLog", singleton);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
