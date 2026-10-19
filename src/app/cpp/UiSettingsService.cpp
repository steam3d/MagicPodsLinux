// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "UiSettingsService.h"

#include <QDebug>

UiSettingsService::UiSettingsService(QObject *parent)
    : QObject(parent)
    , settings(QSettings::IniFormat,
               QSettings::UserScope,
               QStringLiteral("magicpods"),
               QStringLiteral("ui"))
{
}

QString UiSettingsService::fileName() const
{
    return settings.fileName();
}

QVariant UiSettingsService::value(const QString &key) const
{
    return settings.value(key);
}

QVariant UiSettingsService::value(const QString &key, const QVariant &defaultValue) const
{
    return settings.value(key, defaultValue);
}

bool UiSettingsService::setValue(const QString &key, const QVariant &value)
{
    if (key.isEmpty()) {
        qWarning() << "[UiSettingsService] Cannot save a setting with an empty key";
        return false;
    }

    const bool changed = !settings.contains(key) || settings.value(key) != value;
    settings.setValue(key, value);

    if (!sync())
        return false;

    if (changed)
        emit valueChanged(key, value);

    return true;
}

bool UiSettingsService::contains(const QString &key) const
{
    return settings.contains(key);
}

bool UiSettingsService::remove(const QString &key)
{
    if (!settings.contains(key))
        return true;

    settings.remove(key);

    if (!sync())
        return false;

    emit valueChanged(key, {});
    return true;
}

QStringList UiSettingsService::allKeys() const
{
    return settings.allKeys();
}

bool UiSettingsService::sync()
{
    settings.sync();
    if (settings.status() == QSettings::NoError)
        return true;

    qWarning() << "[UiSettingsService] Failed to synchronize" << settings.fileName()
               << "with status" << settings.status();
    return false;
}
