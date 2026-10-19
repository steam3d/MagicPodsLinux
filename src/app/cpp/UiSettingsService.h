// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QVariant>

class UiSettingsService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName CONSTANT)

public:
    explicit UiSettingsService(QObject *parent = nullptr);

    QString fileName() const;

    Q_INVOKABLE QVariant value(const QString &key) const;
    Q_INVOKABLE QVariant value(const QString &key, const QVariant &defaultValue) const;
    Q_INVOKABLE bool setValue(const QString &key, const QVariant &value);
    Q_INVOKABLE bool contains(const QString &key) const;
    Q_INVOKABLE bool remove(const QString &key);
    Q_INVOKABLE QStringList allKeys() const;
    Q_INVOKABLE bool sync();

signals:
    void valueChanged(const QString &key, const QVariant &value);

private:
    QSettings settings;
};
