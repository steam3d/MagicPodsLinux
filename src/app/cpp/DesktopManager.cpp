// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "DesktopManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

DesktopManager::DesktopManager(QObject *parent)
    : QObject(parent) {}

static QString applicationPath()
{
    // Use launcher script path if available (CQtDeployer sets CQT_RUN_FILE),
    // otherwise fall back to the binary itself (dev builds)
    QString path = qEnvironmentVariable("CQT_RUN_FILE");
    if (path.isEmpty())
        path = QCoreApplication::applicationFilePath();
    return path;
}

static QString iconInstallPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
           + "/icons/magicpods.png";
}

static bool installIcon()
{
    const QString iconDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                            + "/icons";
    if (!QDir().mkpath(iconDir))
        return false;

    const QString dest = iconInstallPath();
    if (QFile::exists(dest))
        QFile::remove(dest);

    return QFile::copy(":/qt/qml/magicpods/src/app/qml/assets/images/logo-512.png", dest);
}

bool DesktopManager::createDesktopFile()
{
    if (!installIcon())
        qWarning() << "[DesktopManager] Failed to install icon";

    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                            + QString("/applications");
    qDebug() << dirPath;

    if (!QDir().mkpath(dirPath))
        return false;

    const QString filePath = dirPath + "/app.magicpods.desktop";
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;

    QTextStream out(&f);
    out << QString(
R"([Desktop Entry]
Type=Application
Name=MagicPods
Comment=The control center for your Bluetooth headphones
Exec=%1
Icon=%2
Terminal=false
Categories=Utility;
StartupWMClass=MagicPods
)")
    .arg(applicationPath(), iconInstallPath());
    return true;
}

bool DesktopManager::createAutostartFile()
{
    if (!installIcon())
        qWarning() << "[DesktopManager] Failed to install icon";

    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                            + QString("/autostart");
    if (!QDir().mkpath(dirPath))
        return false;

    QFile f(dirPath + "/app.magicpods.desktop");
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;

    QTextStream out(&f);
    out << QString(
R"([Desktop Entry]
Type=Application
Name=MagicPods
Comment=The control center for your Bluetooth headphones
Exec=%1 --background
Icon=%2
Terminal=false
Categories=Utility;
StartupWMClass=MagicPods
)")
    .arg(applicationPath(), iconInstallPath());
    return true;
}

QString DesktopManager::desktopFilePath() const
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                         + "/applications/app.magicpods.desktop";
    return QFileInfo::exists(path) ? path : QString{};
}

QString DesktopManager::autostartFilePath() const
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                         + "/autostart/app.magicpods.desktop";
    return QFileInfo::exists(path) ? path : QString{};
}

bool DesktopManager::isInstalled() const
{
    return !desktopFilePath().isEmpty();
}

bool DesktopManager::isAutostartInstalled() const
{
    return !autostartFilePath().isEmpty();
}

bool DesktopManager::install()
{
    if (isInstalled()) {
        qDebug() << "[DesktopManager] Desktop file already exists";
        return true;
    }

    qDebug() << "[DesktopManager] Creating desktop file";
    return createDesktopFile();
}

bool DesktopManager::uninstall()
{
    const bool removed = QFile::remove(desktopFilePath());
    if (removed && !isAutostartInstalled())
        QFile::remove(iconInstallPath());
    return removed;
}

bool DesktopManager::installAutostart()
{
    if (isAutostartInstalled()) {
        qDebug() << "[DesktopManager] Autostart file already exists";
        return true;
    }

    qDebug() << "[DesktopManager] Creating autostart file";
    return createAutostartFile();
}

bool DesktopManager::uninstallAutostart()
{
    const bool removed = QFile::remove(autostartFilePath());
    if (removed && !isInstalled())
        QFile::remove(iconInstallPath());
    return removed;
}
