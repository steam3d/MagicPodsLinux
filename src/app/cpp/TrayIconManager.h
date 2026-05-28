// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <functional>

#include <QIcon>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class Backend;
class TrayIcon;
class QMenu;

class TrayIconManager final : public QObject
{
    Q_OBJECT

public:
    explicit TrayIconManager(TrayIcon *trayIcon,
                             QMenu *menu,
                             Backend *backend,
                             std::function<void()> openSettings,
                             std::function<void()> exitApplication,
                             QObject *parent = nullptr);

private:
    void updateState(const QVariantMap &json);
    void handleDataReceived(const QVariant &json);
    void updateTrayIcon();
    void rebuildMenu();
    bool batteryAvailable(const QVariantMap &batteryPart) const;
    int trayBattery() const;
    QString trayTooltipText() const;
    QString composeToolTip(const QString &details) const;
    QList<QVariantMap> sortedHeadphones() const;
    QVariantMap ancDataForAddress(const QString &address) const;
    void addHeadphoneAction(const QVariantMap &headphone);
    void addAncActions(const QString &address, const QVariantMap &ancData);
    void addAncAction(const QString &address,
                      const QVariantMap &ancData,
                      int value,
                      const char *labelId,
                      const char *iconName);
    QIcon iconFromAppAssets(const QString &fileName) const;

    TrayIcon *trayIcon = nullptr;
    QMenu *menu = nullptr;
    Backend *backend = nullptr;
    std::function<void()> openSettings;
    std::function<void()> exitApplication;
    QVariantList headphonesData;
    QVariantMap infoData;
    bool menuOpen = false;
};
