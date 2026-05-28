// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <QObject>
#include <QString>

class DeckyManager : public QObject
{
    Q_OBJECT

public:
    explicit DeckyManager(QObject *parent = nullptr);
    Q_INVOKABLE QString version();

private:
    QString manualMetadataPath() const;
    QString storeMetadataPath() const;
    QString readVersionFromMetadata(const QString &path) const;
};
