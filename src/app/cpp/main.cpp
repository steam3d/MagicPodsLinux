// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include <QApplication>
#include <QQmlApplicationEngine>
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
#include <QSettings>
#include <QStyleHints>
#include <QStringList>
#include <QWindow>

#include "BackendManager.h"
#include "DesktopManager.h"
#include "DeckyManager.h"
#include "GlobalShortcutManager.h"
#include "LanguageManager.h"
#include "ShortcutActionManager.h"
#include "TrayIcon.h"
#include "TrayIconManager.h"
#include "UiSettingsService.h"
#include "Backend.h"

namespace {
const QString GameModeScaleSettingKey = QStringLiteral("GameMode/scale");
const QString DefaultGameModeScale = QStringLiteral("1.5");
const QString ApplicationThemeSettingKey = QStringLiteral("Theme/application");

void applyApplicationTheme(const QString &theme)
{
    Qt::ColorScheme colorScheme = Qt::ColorScheme::Unknown;
    if (theme == QStringLiteral("light"))
        colorScheme = Qt::ColorScheme::Light;
    else if (theme == QStringLiteral("dark"))
        colorScheme = Qt::ColorScheme::Dark;

    QGuiApplication::styleHints()->setColorScheme(colorScheme);
}

void applyGameModeScale()
{
    if (qEnvironmentVariableIsEmpty("GAMESCOPE_WAYLAND_DISPLAY")
        || !qEnvironmentVariableIsEmpty("QT_SCALE_FACTOR")) {
        return;
    }

    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       QStringLiteral("magicpods"),
                       QStringLiteral("ui"));
    const QString scale = settings.value(GameModeScaleSettingKey, DefaultGameModeScale).toString();
    if (scale == QStringLiteral("off"))
        return;

    const QStringList supportedScales{
        QStringLiteral("1.0"),
        QStringLiteral("1.25"),
        QStringLiteral("1.5"),
        QStringLiteral("1.75"),
        QStringLiteral("2.0")
    };
    qputenv("QT_SCALE_FACTOR",
            (supportedScales.contains(scale) ? scale : DefaultGameModeScale).toUtf8());
}
} // namespace

static void ensureEnvDefaults() {
    applyGameModeScale();

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

    // TODO: Package with Qt 6.11.1 or newer for QTBUG-145740, which prevents
    // FluentWinUI3 from drawing keyboard focus frames for programmatic focus.
    QQuickStyle::setStyle("FluentWinUI3");
    app.setApplicationName("MagicPods");
    app.setApplicationDisplayName("MagicPods");
    app.setApplicationVersion(MAGICPODS_VERSION);
    app.setOrganizationName("MagicPods");
    app.setOrganizationDomain("magicpods.app");
    app.setDesktopFileName("app.magicpods");
    app.setQuitOnLastWindowClosed(false);

    const bool startInBackground = app.arguments().contains(QStringLiteral("--background"));

    const QString serverName = QString("magicpods_%1_%2")
                                   .arg(app.organizationDomain(), app.applicationName());
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(150)) {
        if (!startInBackground) {
            socket.write("raise");
            socket.flush();
            socket.waitForBytesWritten(150);
        }
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
    auto toggleMainWindow = [&]() {
        if (!root)
            return;

        if (root->property("visible").toBool())
            root->setProperty("visible", false);
        else
            raiseWindow();
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

    UiSettingsService uiSettings;
    const QString applicationTheme = uiSettings
                                         .value(ApplicationThemeSettingKey,
                                                QStringLiteral("system"))
                                         .toString();
    applyApplicationTheme(applicationTheme);
    QObject::connect(&uiSettings,
                     &UiSettingsService::valueChanged,
                     &app,
                     [](const QString &key, const QVariant &value) {
                         if (key == ApplicationThemeSettingKey)
                             applyApplicationTheme(value.toString());
                     });

    LanguageManager languageManager(&uiSettings);
    languageManager.installTranslators();

    const bool gameScopeMode = !qEnvironmentVariableIsEmpty("GAMESCOPE_WAYLAND_DISPLAY");

    BackendManager backendManager;
    DesktopManager desktopManager;
    DeckyManager deckyManager;
    GlobalShortcutManager globalShortcutManager(gameScopeMode,
                                                 desktopManager.isInstalled());
    Backend backend;
    ShortcutActionManager shortcutActionManager(&backend,
                                                 &uiSettings,
                                                 toggleMainWindow);
    QObject::connect(&globalShortcutManager,
                     &GlobalShortcutManager::shortcutActivated,
                     &shortcutActionManager,
                     &ShortcutActionManager::executeShortcut);
    QObject::connect(&languageManager,
                     &LanguageManager::languageChanged,
                     &shortcutActionManager,
                     &ShortcutActionManager::refreshTranslations);
    TrayIcon trayIcon;
    const QString trayIconThemeKey = QStringLiteral("Theme/tray_icon");
    const QString trayIconTheme = uiSettings
                                      .value(trayIconThemeKey, QStringLiteral("auto"))
                                      .toString();
    if (trayIconTheme == QStringLiteral("light"))
        trayIcon.setThemeMode(1);
    else if (trayIconTheme == QStringLiteral("dark"))
        trayIcon.setThemeMode(2);
    else
        trayIcon.setThemeMode(0);

    QQmlApplicationEngine engine;
    languageManager.setEngine(&engine);
    engine.rootContext()->setContextProperty("backendManager", &backendManager);
    engine.rootContext()->setContextProperty("desktopManager", &desktopManager);
    engine.rootContext()->setContextProperty("deckyManager", &deckyManager);
    engine.rootContext()->setContextProperty("globalShortcutManager", &globalShortcutManager);
    engine.rootContext()->setContextProperty("shortcutActionManager", &shortcutActionManager);
    engine.rootContext()->setContextProperty("cppBackend", &backend);
    engine.rootContext()->setContextProperty("cppTrayIcon", &trayIcon);
    engine.rootContext()->setContextProperty("uiSettings", &uiSettings);
    engine.rootContext()->setContextProperty("languageManager", &languageManager);
    engine.rootContext()->setContextProperty("gameScopeMode", gameScopeMode);
    engine.rootContext()->setContextProperty("startInBackground", startInBackground);
    globalShortcutManager.start();
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
    TrayIconManager trayIconManager(&trayIcon, &trayMenu, &backend, toggleMainWindow, [&app]() {
        app.quit();
    });
    QObject::connect(&languageManager,
                     &LanguageManager::languageChanged,
                     &trayIconManager,
                     &TrayIconManager::retranslate);

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
