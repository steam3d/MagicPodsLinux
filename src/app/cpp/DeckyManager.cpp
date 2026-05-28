// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "DeckyManager.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

DeckyManager::DeckyManager(QObject *parent)
    : QObject(parent) {}

QString DeckyManager::manualMetadataPath() const
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                         + "/homebrew/plugins/MagicPods/package.json";
    return QFileInfo::exists(path) ? path : QString{};
}

QString DeckyManager::storeMetadataPath() const
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                         + "/homebrew/plugins/MagicPodsDecky/package.json";
    return QFileInfo::exists(path) ? path : QString{};
}

QString DeckyManager::readVersionFromMetadata(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return {};

    const QJsonValue v = doc.object().value(QStringLiteral("version"));
    return v.isString() ? v.toString() : QString{};
}

QString DeckyManager::version()
{
    const QString store = readVersionFromMetadata(storeMetadataPath());
    const QString manual = readVersionFromMetadata(manualMetadataPath());

    if (!store.isEmpty() && !manual.isEmpty())
        return store + "/" + manual;
    if (!store.isEmpty())
        return store;
    return manual;
}
