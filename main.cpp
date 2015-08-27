#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include <QDebug>

#include "TimeLogModel.h"
#include "ReverseProxyModel.h"
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
    ReverseProxyModel *proxy = new ReverseProxyModel;
    proxy->setSourceModel(model);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("TimeLogModel", proxy);
    engine.rootContext()->setContextProperty("TimeLog", singleton);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
