#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "PackerController.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("Repack Studio");
    QGuiApplication::setOrganizationName("RepackSuite");
    QQuickStyle::setStyle("Basic");

    qmlRegisterType<PackerController>("RepackStudio", 1, 0, "PackerController");

    QQmlApplicationEngine engine;
    engine.loadFromModule("RepackStudio", "Main");
    if (engine.rootObjects().isEmpty()) return 1;
    return app.exec();
}

