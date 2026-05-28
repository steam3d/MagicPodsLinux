// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <QObject>
#include <QString>

class DesktopManager : public QObject
{
    Q_OBJECT

public:
    explicit DesktopManager(QObject *parent = nullptr);
    Q_INVOKABLE bool install();
    Q_INVOKABLE bool uninstall();
    Q_INVOKABLE bool isInstalled() const;

private:
    QString desktopFilePath() const;
    bool createDesktopFile();
};
