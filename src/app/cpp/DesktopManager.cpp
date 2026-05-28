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

    // Use launcher script path if available (CQtDeployer sets CQT_RUN_FILE),
    // otherwise fall back to the binary itself (dev builds)
    QString exePath = qEnvironmentVariable("CQT_RUN_FILE");
    if (exePath.isEmpty())
        exePath = QCoreApplication::applicationFilePath();

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
    .arg(exePath, iconInstallPath());
    return true;
}

QString DesktopManager::desktopFilePath() const
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                         + "/applications/app.magicpods.desktop";
    return QFileInfo::exists(path) ? path : QString{};
}

bool DesktopManager::isInstalled() const
{
    return !desktopFilePath().isEmpty();
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
    QFile::remove(iconInstallPath());
    return QFile::remove(desktopFilePath());
}
