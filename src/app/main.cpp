#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QtQml/QQmlContext>

#include <QLoggingCategory>

#include "FileLogger.h"
#include "TimeLogHistory.h"
#include "TimeLogRecentModel.h"
#include "TimeLogSearchModel.h"
#include "ReverseProxyModel.h"
#include "TimeLogCategoryTreeModel.h"
#include "TimeLogCategoryDepthModel.h"
#include "TimeTracker.h"
#include "TimeLogCategory.h"
#include "DataImporter.h"
#include "DataExporter.h"
#include "DataSyncer.h"

Q_LOGGING_CATEGORY(MAIN_CATEGORY, "main", QtInfoMsg)

static QObject *timeTrackerSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new TimeTracker();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("G-TimeTracker");
    app.setOrganizationDomain("g-timetracker.org");
    app.setApplicationName("G-TimeTracker");
    app.setApplicationVersion("0.1");

    qSetMessagePattern("[%{time}] <%{category}> %{type} (%{file}:%{line}, %{function}) %{message}");
    FileLogger::setup();

    QCommandLineParser parser;
    parser.setApplicationDescription("Global Time Tracker");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption importOption(QStringList() << "i" << "import", "Import a CSV file(s)",
                                    "file or directory");
    parser.addOption(importOption);
    QCommandLineOption exportOption(QStringList() << "e" << "export", "Export to a CSV file(s)",
                                    "target directory");
    parser.addOption(exportOption);
    QCommandLineOption separatorOption("separator", "Separator for import/export", "string", ";");
    parser.addOption(separatorOption);
    QCommandLineOption dataPathOption("dataPath", "Use specified path to program's data", "path");
    parser.addOption(dataPathOption);
    QCommandLineOption syncPathOption("syncPath", "Override path to sync folder", "path");
    parser.addOption(syncPathOption);

    parser.process(app);

    qRegisterMetaType<TimeLogData>();
    qRegisterMetaType<TimeLogEntry>();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<QVector<TimeLogStats> >();
    qRegisterMetaType<QVector<TimeLogSyncData> >();
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();
    qRegisterMetaType<QSharedPointer<TimeLogCategory> >();
    qRegisterMetaType<QMap<QDateTime,QByteArray> >();

    if (parser.isSet(importOption)) {
        TimeLogHistory history;
        if (!history.init(parser.value(dataPathOption))) {
            qCCritical(MAIN_CATEGORY) << "Fail to initialize db";
            return EXIT_FAILURE;
        }

        DataImporter importer(&history);
        importer.setSeparator(parser.value(separatorOption));
        importer.start(parser.value(importOption));
        return app.exec();
    } else if (parser.isSet(exportOption)) {
        TimeLogHistory history;
        if (!history.init(parser.value(dataPathOption))) {
            qCCritical(MAIN_CATEGORY) << "Fail to initialize db";
            return EXIT_FAILURE;
        }

        DataExporter exporter(&history);
        exporter.setSeparator(parser.value(separatorOption));
        exporter.start(parser.value(exportOption));
        return app.exec();
    } else {
        qmlRegisterSingletonType<TimeTracker>("TimeLog", 1, 0, "TimeTracker", timeTrackerSingletonTypeProvider);
        qmlRegisterType<TimeLogModel>("TimeLog", 1, 0, "TimeLogModel");
        qmlRegisterType<TimeLogRecentModel>("TimeLog", 1, 0, "TimeLogRecentModel");
        qmlRegisterType<TimeLogSearchModel>("TimeLog", 1, 0, "TimeLogSearchModel");
        qmlRegisterType<ReverseProxyModel>("TimeLog", 1, 0, "ReverseProxyModel");
        qmlRegisterType<TimeLogCategoryTreeModel>("TimeLog", 1, 0, "TimeLogCategoryTreeModel");
        qmlRegisterType<TimeLogCategoryDepthModel>("TimeLog", 1, 0, "TimeLogCategoryDepthModel");
        qmlRegisterUncreatableType<DataSyncer>("TimeLog", 1, 0, "DataSyncer", "This is a DataSyncer object");
        qmlRegisterSingletonType(QUrl("qrc:/qml/timetracker/Settings.qml"), "TimeLog", 1, 0, "Settings");

        if (!qputenv("QT_LABS_CONTROLS_STYLE", "material")) {
            qCCritical(MAIN_CATEGORY) << "Fail to set style";
        }

        QQmlApplicationEngine engine;
        engine.addImportPath(QString("%1/qml").arg(app.applicationDirPath()));
        engine.rootContext()->setContextProperty("TimeLogDataPath", parser.value(dataPathOption));
        engine.rootContext()->setContextProperty("TimeLogSyncPath", parser.value(syncPathOption));
        engine.load(QUrl(QStringLiteral("qrc:/qml/timetracker/main.qml")));
        return app.exec();
    }
}
