#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include "InstallerController.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("Game Installer");
    QGuiApplication::setOrganizationName("RepackSuite");
    QQuickStyle::setStyle("Basic");

    qmlRegisterType<InstallerController>("RepackInstaller", 1, 0, "InstallerController");

    QQmlApplicationEngine engine;
    engine.loadFromModule("RepackInstaller", "Main");
    if (engine.rootObjects().isEmpty()) return 1;
    return app.exec();
}

