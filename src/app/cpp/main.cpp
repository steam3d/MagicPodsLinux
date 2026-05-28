// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QLibraryInfo>
#include <QQmlContext>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QAction>
#include <QFontDatabase>
#include <QMenu>
#include <QQmlComponent>
#include <QQuickStyle>
#include <QScopedPointer>
#include <QWindow>

#include "BackendManager.h"
#include "DesktopManager.h"
#include "DeckyManager.h"
#include "TrayIcon.h"
#include "TrayIconManager.h"
#include "Backend.h"

static void ensureEnvDefaults() {
    if (qEnvironmentVariableIsEmpty("QML2_IMPORT_PATH")) {
        const QString qmlPath = QLibraryInfo::path(QLibraryInfo::QmlImportsPath);
        if (!qmlPath.isEmpty()) {
            qputenv("QML2_IMPORT_PATH", qmlPath.toUtf8());
        }
    }

    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        const bool hasWayland = !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY");
        qputenv("QT_QPA_PLATFORM", hasWayland ? "wayland;xcb" : "xcb");
    }


    if (qEnvironmentVariableIsEmpty("QML_XHR_ALLOW_FILE_READ")) {
        qputenv("QML_XHR_ALLOW_FILE_READ", "1");
    }

}

int main(int argc, char *argv[]) {
    ensureEnvDefaults();

    QApplication app(argc, argv);

    QFontDatabase::addApplicationFont(":/fonts/NotoSans-Thin.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-ThinItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-ExtraLight.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-ExtraLightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-Light.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-LightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-Medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-MediumItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-SemiBold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-SemiBoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-ExtraBold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-ExtraBoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-Black.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSans-BlackItalic.ttf");
    app.setFont(QFont("Noto Sans", app.font().pointSize()));

    QQuickStyle::setStyle("FluentWinUI3");
    app.setApplicationName("MagicPods");
    app.setApplicationDisplayName("MagicPods");
    app.setApplicationVersion(MAGICPODS_VERSION);
    app.setOrganizationName("MagicPods");
    app.setOrganizationDomain("magicpods.app");
    app.setDesktopFileName("app.magicpods");
    app.setQuitOnLastWindowClosed(false);

    const QString serverName = QString("magicpods_%1_%2")
                                   .arg(app.organizationDomain(), app.applicationName());
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(150)) {
        socket.write("raise");
        socket.flush();
        socket.waitForBytesWritten(150);
        return 0;
    }

    QLocalServer::removeServer(serverName);
    QLocalServer server;
    if (!server.listen(serverName)) {
        qWarning() << "Failed to listen on local server:" << server.errorString();
    }

    QObject *root = nullptr;
    bool raisePending = false;
    auto raiseWindow = [&]() {
        if (!root) {
            raisePending = true;
            return;
        }
        if (auto window = qobject_cast<QWindow *>(root)) {
            window->show();
            window->raise();
            window->requestActivate();
        } else {
            root->setProperty("visible", true);
        }
    };
    auto handleRaiseRequest = [&]() {
        auto client = server.nextPendingConnection();
        if (client) {
            client->readAll();
            client->disconnectFromServer();
            client->deleteLater();
        }
        raiseWindow();
    };

    QObject::connect(&server, &QLocalServer::newConnection, handleRaiseRequest);
    while (server.hasPendingConnections()) {
        handleRaiseRequest();
    }

    QIcon appIcon;
    appIcon.addFile(QStringLiteral(":/qt/qml/magicpods/src/app/qml/assets/images/logo-16.png"),  QSize(16, 16));
    appIcon.addFile(QStringLiteral(":/qt/qml/magicpods/src/app/qml/assets/images/logo-32.png"),  QSize(32, 32));
    appIcon.addFile(QStringLiteral(":/qt/qml/magicpods/src/app/qml/assets/images/logo-48.png"),  QSize(48, 48));
    appIcon.addFile(QStringLiteral(":/qt/qml/magicpods/src/app/qml/assets/images/logo-256.png"), QSize(256, 256));
    appIcon.addFile(QStringLiteral(":/qt/qml/magicpods/src/app/qml/assets/images/logo-512.png"), QSize(512, 512));
    app.setWindowIcon(appIcon);

    QTranslator englishTranslation;
    const QString englishTranslationPath = QStringLiteral(":/i18n/locale_en.qm");
    if (!englishTranslation.load(englishTranslationPath)) {
        qFatal("Could not load %s!", qUtf8Printable(englishTranslationPath));
    }
    app.installTranslator(&englishTranslation);

    QTranslator localizedTranslation;
    if (QLocale().language() != QLocale::English
        && localizedTranslation.load(QLocale(),
                                     QStringLiteral("locale"),
                                     QStringLiteral("_"),
                                     QStringLiteral(":/i18n/"))) {
        app.installTranslator(&localizedTranslation);
    }

    const bool gameScopeMode = !qEnvironmentVariableIsEmpty("GAMESCOPE_WAYLAND_DISPLAY");

    QQmlApplicationEngine engine;
    BackendManager backendManager;
    DesktopManager desktopManager;
    DeckyManager deckyManager;
    Backend backend;
    TrayIcon trayIcon;
    engine.rootContext()->setContextProperty("backendManager", &backendManager);
    engine.rootContext()->setContextProperty("desktopManager", &desktopManager);
    engine.rootContext()->setContextProperty("deckyManager", &deckyManager);
    engine.rootContext()->setContextProperty("cppBackend", &backend);
    engine.rootContext()->setContextProperty("cppTrayIcon", &trayIcon);
    engine.rootContext()->setContextProperty("gameScopeMode", gameScopeMode);
    backendManager.start();
    backend.connectSocket();
    engine.loadFromModule("magicpods", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    QQmlComponent popupAnimationComponent(
        &engine,
        QUrl(QStringLiteral("qrc:/qt/qml/magicpods/src/app/qml/PopupAnimation.qml"))
    );
    if (popupAnimationComponent.isError()) {
        qWarning() << popupAnimationComponent.errors();
    }
    QScopedPointer<QObject> popupAnimation(popupAnimationComponent.create(engine.rootContext()));
    if (!popupAnimation) {
        qWarning() << "Failed to create PopupAnimation.qml";
        qWarning() << popupAnimationComponent.errors();
    }

    root = engine.rootObjects().constFirst();
    if (raisePending) {
        raiseWindow();
    }

    QMenu trayMenu;
    auto toggleMainWindow = [&]() {
        if (!root) {
            return;
        }
        if (auto window = qobject_cast<QWindow *>(root)) {
            const bool visible = window->isVisible();
            if (visible) {
                window->hide();
            } else {
                window->show();
                window->raise();
                window->requestActivate();
            }
        } else {
            const bool visible = root->property("visible").toBool();
            root->setProperty("visible", !visible);
        }
    };
    TrayIconManager trayIconManager(&trayIcon, &trayMenu, &backend, toggleMainWindow, [&app]() {
        app.quit();
    });

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        trayIcon.setContextMenu(&trayMenu);
        QObject::connect(&trayIcon, &TrayIcon::leftClicked, [&]() {
            toggleMainWindow();
        });

        trayIcon.show();
    }

    const QMetaObject::Connection backendRecoveryConnection =
        QObject::connect(&backend, &Backend::connectedChanged, &backendManager, [&backend, &backendManager]() {
        if (!backend.connected())
            backendManager.recover();
    });

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &backendManager, [&backend, &backendManager, backendRecoveryConnection]() {
        QObject::disconnect(backendRecoveryConnection);
        backendManager.beginShutdown();
        backend.disconnectSocket();
        backendManager.stop();
    });

    return app.exec();
}
