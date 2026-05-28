// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "TrayIconManager.h"

#include "Backend.h"
#include "TrayIcon.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QStringList>
#include <QtGlobal>
#include <algorithm>

namespace {
struct AncModeDefinition {
    int value;
    const char *labelId;
    const char *iconName;
};

constexpr int kAncOff = 1;
constexpr int kAncTransparency = 2;
constexpr int kAncAdaptive = 4;
constexpr int kAncWind = 8;
constexpr int kAncNoiseCancellation = 16;

const AncModeDefinition kAncModes[] = {
    {kAncOff, "battery.anc_off", "icon-off.svg"},
    {kAncTransparency, "battery.anc_tra", "icon-tra.svg"},
    {kAncAdaptive, "battery.anc_adaptive", "icon-adaptive.svg"},
    {kAncWind, "battery.anc_wind", "icon-wind.svg"},
    {kAncNoiseCancellation, "battery.anc_anc", "icon-noise.svg"},
};

QVariantMap batteryDataFromInfo(const QVariantMap &infoData)
{
    return infoData.value(QStringLiteral("capabilities")).toMap().value(QStringLiteral("battery")).toMap();
}

}

TrayIconManager::TrayIconManager(TrayIcon *trayIcon,
                                 QMenu *menu,
                                 Backend *backend,
                                 std::function<void()> openSettings,
                                 std::function<void()> exitApplication,
                                 QObject *parent)
    : QObject(parent)
    , trayIcon(trayIcon)
    , menu(menu)
    , backend(backend)
    , openSettings(std::move(openSettings))
    , exitApplication(std::move(exitApplication))
{
    Q_ASSERT(this->trayIcon);
    Q_ASSERT(this->menu);
    Q_ASSERT(this->backend);

    connect(this->backend, &Backend::dataReceived, this, &TrayIconManager::handleDataReceived);
    connect(this->backend, &Backend::connectedChanged, this, [this]() {

        if (!this->backend->connected()) {
            headphonesData.clear();
            infoData.clear();
        } else {
            this->backend->getAll();
        }
        updateTrayIcon();
        rebuildMenu();
    });
    connect(this->menu, &QMenu::aboutToShow, this, [this]() {
        menuOpen = true;
        rebuildMenu();
    });
    connect(this->menu, &QMenu::aboutToHide, this, [this]() {
        menuOpen = false;
    });

    updateTrayIcon();
    rebuildMenu();
}

void TrayIconManager::handleDataReceived(const QVariant &json)
{
    updateState(json.toMap());
    updateTrayIcon();
    rebuildMenu();
}

void TrayIconManager::updateState(const QVariantMap &json)
{
    if (json.isEmpty()) {
        headphonesData.clear();
        infoData.clear();
        return;
    }

    const auto headphonesIt = json.find(QStringLiteral("headphones"));
    if (headphonesIt != json.end()) {
        headphonesData = headphonesIt->toList();
    }

    const auto infoIt = json.find(QStringLiteral("info"));
    if (infoIt != json.end()) {
        infoData = infoIt->toMap();
    }
}

void TrayIconManager::updateTrayIcon()
{
    if (!trayIcon || !backend) {
        return;
    }

    const QVariantMap batteryData = batteryDataFromInfo(infoData);
    if (!backend->connected()) {
        trayIcon->setIconType(TrayIcon::IconType::Warning);
    } else if (batteryData.isEmpty()) {
        trayIcon->setIconType(TrayIcon::IconType::Default);
    } else {
        trayIcon->setTextIcon(trayBattery());
    }

    trayIcon->setToolTip(composeToolTip(trayTooltipText()));
}

void TrayIconManager::rebuildMenu()
{
    if (!menu) {
        return;
    }

    if (menuOpen) {
        qDebug() << "rebuildMenu: skipped, menu is open";
        return;
    }

    menu->clear();

    bool hasDynamicItems = false;
    if (backend->connected()) {
        const QList<QVariantMap> headphones = sortedHeadphones();
        for (const QVariantMap &headphone : headphones) {
            addHeadphoneAction(headphone);
            hasDynamicItems = true;

            const QString address = headphone.value(QStringLiteral("address")).toString();
            const QVariantMap ancData = ancDataForAddress(address);
            if (!ancData.isEmpty()) {
                menu->addSeparator();
                addAncActions(address, ancData);
                menu->addSeparator();
            }
        }
    }

    if (hasDynamicItems) {
        menu->addSeparator();
    }

    QAction *settingsAction = menu->addAction(qtTrId("menu.settings"));
    connect(settingsAction, &QAction::triggered, this, [this]() {
        if (openSettings) {
            openSettings();
        }
    });

    QAction *exitAction = menu->addAction(qtTrId("tray.exit"));
    connect(exitAction, &QAction::triggered, this, [this]() {
        if (exitApplication) {
            exitApplication();
        }
    });
}

bool TrayIconManager::batteryAvailable(const QVariantMap &batteryPart) const
{
    const int status = batteryPart.value(QStringLiteral("status")).toInt();
    return status == 2 || status == 3;
}

int TrayIconManager::trayBattery() const
{
    const QVariantMap batteryData = batteryDataFromInfo(infoData);
    if (batteryData.isEmpty()) {
        return 0;
    }

    const QVariantMap single = batteryData.value(QStringLiteral("single")).toMap();
    if (batteryAvailable(single)) {
        return single.value(QStringLiteral("battery")).toInt();
    }

    const QVariantMap left = batteryData.value(QStringLiteral("left")).toMap();
    const QVariantMap right = batteryData.value(QStringLiteral("right")).toMap();
    const bool hasLeft = batteryAvailable(left);
    const bool hasRight = batteryAvailable(right);

    if (hasLeft && hasRight) {
        return qRound((left.value(QStringLiteral("battery")).toInt()
                       + right.value(QStringLiteral("battery")).toInt()) / 2.0);
    }
    if (hasLeft) {
        return left.value(QStringLiteral("battery")).toInt();
    }
    if (hasRight) {
        return right.value(QStringLiteral("battery")).toInt();
    }
    return 0;
}

QString TrayIconManager::trayTooltipText() const
{
    if (!backend->connected()) {
        return qtTrId("tray.socket_error_tooltip");
    }

    const QVariantMap batteryData = batteryDataFromInfo(infoData);
    if (batteryData.isEmpty()) {
        return qtTrId("tray.disconnected");
    }

    QStringList parts;
    const auto appendBattery = [this, &parts](const QVariantMap &batteryPart, const QString &label) {
        if (!batteryAvailable(batteryPart)) {
            return;
        }
        parts.append(QStringLiteral("%1: %2%").arg(label).arg(batteryPart.value(QStringLiteral("battery")).toInt()));
    };

    appendBattery(batteryData.value(QStringLiteral("single")).toMap(),
                  qtTrId("battery.battery_single"));
    appendBattery(batteryData.value(QStringLiteral("left")).toMap(),
                  qtTrId("battery.battery_left"));
    appendBattery(batteryData.value(QStringLiteral("right")).toMap(),
                  qtTrId("battery.battery_right"));
    appendBattery(batteryData.value(QStringLiteral("case")).toMap(),
                  qtTrId("battery.battery_case"));

    if (parts.isEmpty()) {
        return qtTrId("tray.disconnected");
    }

    return parts.join(QLatin1Char(' '));
}

QString TrayIconManager::composeToolTip(const QString &details) const
{
    const QString appName = QApplication::applicationDisplayName().isEmpty()
        ? QGuiApplication::applicationName()
        : QApplication::applicationDisplayName();

    if (details.isEmpty()) {
        return appName;
    }

    return QStringLiteral("%1\n%2").arg(appName, details);
}

QList<QVariantMap> TrayIconManager::sortedHeadphones() const
{
    QList<QVariantMap> headphones;
    headphones.reserve(headphonesData.size());
    for (const QVariant &value : headphonesData) {
        headphones.append(value.toMap());
    }

    std::sort(headphones.begin(), headphones.end(), [](const QVariantMap &lhs, const QVariantMap &rhs) {
        return lhs.value(QStringLiteral("name")).toString().localeAwareCompare(
                   rhs.value(QStringLiteral("name")).toString())
               < 0;
    });

    return headphones;
}

QVariantMap TrayIconManager::ancDataForAddress(const QString &address) const
{
    if (address.isEmpty() || infoData.value(QStringLiteral("address")).toString() != address) {
        return {};
    }

    return infoData.value(QStringLiteral("capabilities")).toMap().value(QStringLiteral("anc")).toMap();
}

void TrayIconManager::addHeadphoneAction(const QVariantMap &headphone)
{
    const QString address = headphone.value(QStringLiteral("address")).toString();
    const QString name = headphone.value(QStringLiteral("name")).toString();
    const bool connected = headphone.value(QStringLiteral("connected")).toBool();
    const QString actionText = QStringLiteral("%1 %2")
                                   .arg(connected ? qtTrId("tray.disconnect") : qtTrId("tray.connect"), name);

    QAction *action = menu->addAction(actionText);
    if (connected) {
        action->setIcon(iconFromAppAssets(QStringLiteral("icon-disconnect.svg")));
    }

    connect(action, &QAction::triggered, this, [this, address, connected]() {
        if (connected) {
            backend->disconnectDevice(address);
        } else {
            backend->connectDevice(address);
        }
    });
}

void TrayIconManager::addAncActions(const QString &address, const QVariantMap &ancData)
{
    for (const AncModeDefinition &mode : kAncModes) {
        addAncAction(address, ancData, mode.value, mode.labelId, mode.iconName);
    }
}

void TrayIconManager::addAncAction(const QString &address,
                                   const QVariantMap &ancData,
                                   int value,
                                   const char *labelId,
                                   const char *iconName)
{
    const int options = ancData.value(QStringLiteral("options")).toInt();
    if ((options & value) == 0) {
        return;
    }

    QAction *action = menu->addAction(qtTrId(labelId));

    const int selected = ancData.value(QStringLiteral("selected")).toInt();
    const bool isSelected = selected == value;
    const bool isReadonly = ancData.value(QStringLiteral("readonly")).toBool();
    action->setEnabled(!isSelected && !isReadonly);
    if (isSelected) {
        action->setIcon(iconFromAppAssets(QString::fromLatin1(iconName)));
    }

    connect(action, &QAction::triggered, this, [this, address, value]() {
        backend->setAnc(address, value);
    });
}

QIcon TrayIconManager::iconFromAppAssets(const QString &fileName) const
{
    return QIcon(QStringLiteral(":/qt/qml/magicpods/src/app/qml/assets/icons/") + fileName);
}
