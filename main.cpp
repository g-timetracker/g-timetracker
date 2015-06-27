#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include "TimeLogModel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TimeLogModel *model = new TimeLogModel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("TimeLogModel", model);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
