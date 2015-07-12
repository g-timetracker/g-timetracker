#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include <QDebug>

#include "TimeLogModel.h"
#include "TimeLogSingleton.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("G-TimeTracker");
    app.setOrganizationDomain("g-timetracker.org");
    app.setApplicationName("G-TimeTracker");

    TimeLogSingleton *singleton = new TimeLogSingleton;
    qRegisterMetaType<TimeLogData>();
    TimeLogModel *model = new TimeLogModel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("TimeLogModel", model);
    engine.rootContext()->setContextProperty("TimeLog", singleton);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
