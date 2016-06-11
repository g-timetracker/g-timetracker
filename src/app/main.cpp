/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QtQml/QQmlContext>
#include <QPointer>

#ifndef Q_OS_ANDROID
# include <QFontDatabase>
#endif

#ifndef Q_OS_ANDROID
# include <QtSingleApplication>
#endif

#include <QLoggingCategory>

#include "FileLogger.h"
#include "TimeLogHistory.h"
#include "TimeLogRecentModel.h"
#include "TimeLogSearchModel.h"
#include "ReverseProxyModel.h"
#include "TimeLogCategoryTreeModel.h"
#include "TimeLogCategoryDepthModel.h"
#include "TimeTracker.h"
#include "TimeLogCategoryTreeNode.h"
#include "DataImporter.h"
#include "DataExporter.h"
#include "DataSyncer.h"
#include "Notifier.h"
#ifndef Q_OS_ANDROID
# include "Updater.h"
#endif

Q_LOGGING_CATEGORY(MAIN_CATEGORY, "main", QtInfoMsg)

QPointer<Notifier> mainNotifier;

#ifndef Q_OS_ANDROID
void loadFont(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(MAIN_CATEGORY) << "Fail to open font file" << path << file.errorString();
        return;
    }
    QByteArray data = file.readAll();
    int id = QFontDatabase::addApplicationFontFromData(data);
    if (id == -1) {
        qCWarning(MAIN_CATEGORY) << "Fail to load font" << path;
    }
}
#endif

static QObject *timeTrackerSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    TimeTracker *timetracker = new TimeTracker();
    QObject::connect(mainNotifier, SIGNAL(activateRequested()),
                     timetracker, SIGNAL(activateRequested()));

    return timetracker;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#ifndef Q_OS_ANDROID
    QtSingleApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
#endif
    app.setOrganizationName("G-TimeTracker");
    app.setOrganizationDomain("g-timetracker.org");
    app.setApplicationName("G-TimeTracker");
    app.setApplicationVersion("0.5.2");

    QTranslator translator_en;
    translator_en.load("timetracker_en", ":/translations/");
    app.installTranslator(&translator_en);

    QTranslator translator;
    if (QLocale::system().language() != QLocale::English) {
        translator.load(QLocale::system(), "timetracker", "_", ":/translations/");
        app.installTranslator(&translator);
    }

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
    QCommandLineOption multiOption("multi", "Allow start of multiple instances");
    multiOption.setHidden(true);
    parser.addOption(multiOption);

    parser.process(app);

    Notifier notifier;
    mainNotifier = &notifier;

#ifndef Q_OS_ANDROID
    if (!parser.isSet(multiOption)) {
        if (app.isRunning()) {
            qCInfo(MAIN_CATEGORY) << "The application is already running";
            if (parser.isSet(importOption) || parser.isSet(exportOption)) {
                return EXIT_FAILURE;
            } else {
                app.sendMessage(QString());
                return EXIT_SUCCESS;
            }
        } else {
            QObject::connect(&app, SIGNAL(messageReceived(QString)),
                             &notifier, SLOT(requestActivate()));
        }
    }
#endif

    qRegisterMetaType<TimeLogData>();
    qRegisterMetaType<TimeLogEntry>();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<QVector<TimeLogStats> >();
    qRegisterMetaType<QVector<TimeLogSyncDataEntry> >();
    qRegisterMetaType<QVector<TimeLogSyncDataCategory> >();
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();
    qRegisterMetaType<QSharedPointer<TimeLogCategoryTreeNode> >();
    qRegisterMetaType<QMap<QDateTime,QByteArray> >();
    qRegisterMetaType<TimeLogCategoryData>();
    qRegisterMetaType<TimeLogCategory>();
    qRegisterMetaType<QVector<TimeLogCategory> >();

    if (parser.isSet(importOption)) {
        TimeLogHistory history;
        if (!history.init(parser.value(dataPathOption), QString(), false, true)) {
            qCCritical(MAIN_CATEGORY) << "Fail to initialize db";
            return EXIT_FAILURE;
        }

        DataImporter importer(&history);
        importer.setSeparator(parser.value(separatorOption));
        importer.start(parser.value(importOption));
        return app.exec();
    } else if (parser.isSet(exportOption)) {
        TimeLogHistory history;
        if (!history.init(parser.value(dataPathOption), QString(), true)) {
            qCCritical(MAIN_CATEGORY) << "Fail to initialize db";
            return EXIT_FAILURE;
        }

        DataExporter exporter(&history);
        exporter.setSeparator(parser.value(separatorOption));
        exporter.start(parser.value(exportOption));
        return app.exec();
    } else {
#ifndef Q_OS_ANDROID    // Android devices should has Roboto fonts
        loadFont(":/fonts/Roboto-Regular.ttf");
        loadFont(":/fonts/Roboto-Medium.ttf");
#endif

        qmlRegisterSingletonType<TimeTracker>("TimeLog", 1, 0, "TimeTracker", timeTrackerSingletonTypeProvider);
        qmlRegisterType<TimeLogModel>("TimeLog", 1, 0, "TimeLogModel");
        qmlRegisterType<TimeLogRecentModel>("TimeLog", 1, 0, "TimeLogRecentModel");
        qmlRegisterType<TimeLogSearchModel>("TimeLog", 1, 0, "TimeLogSearchModel");
        qmlRegisterType<ReverseProxyModel>("TimeLog", 1, 0, "ReverseProxyModel");
        qmlRegisterType<TimeLogCategoryTreeModel>("TimeLog", 1, 0, "TimeLogCategoryTreeModel");
        qmlRegisterType<TimeLogCategoryDepthModel>("TimeLog", 1, 0, "TimeLogCategoryDepthModel");
        qmlRegisterUncreatableType<DataSyncer>("TimeLog", 1, 0, "DataSyncer", "This is a DataSyncer object");
#ifndef Q_OS_ANDROID
        qmlRegisterType<Updater>("TimeLog", 1, 0, "Updater");
#endif
        qmlRegisterSingletonType(QUrl("qrc:/qml/timetracker/AppSettings.qml"), "TimeLog", 1, 0, "AppSettings");
        qmlRegisterSingletonType(QUrl("qrc:/qml/timetracker/PlatformMaterial.qml"), "TimeLog", 1, 0, "PlatformMaterial");

        QQmlApplicationEngine engine;
//        QQmlFileSelector::get(&engine)->setExtraSelectors(QStringList() << "desktopStyle");
//        QQmlFileSelector::get(&engine)->setExtraSelectors(QStringList() << "android");
        engine.addImportPath(QString("%1/qml").arg(app.applicationDirPath()));
        engine.rootContext()->setContextProperty("TimeLogDataPath", QUrl::fromLocalFile(parser.value(dataPathOption)));
        engine.rootContext()->setContextProperty("TimeLogSyncPath", QUrl::fromLocalFile(parser.value(syncPathOption)));
        engine.load(QUrl(QStringLiteral("qrc:/qml/timetracker/main.qml")));
        return app.exec();
    }
}
