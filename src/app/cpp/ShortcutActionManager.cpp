// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "ShortcutActionManager.h"

#include "Backend.h"
#include "UiSettingsService.h"

#include <array>
#include <utility>

#include <QCoreApplication>
#include <QSet>
#include <QStringList>

namespace {
constexpr int kShortcutSlotCount = 10;
constexpr qint64 kActivationQuietPeriodMs = 300;

QString shortcutId(int slotNumber)
{
    return QStringLiteral("slot-%1").arg(slotNumber, 2, 10, QLatin1Char('0'));
}

QString settingKey(const QString &slotId, const QString &name)
{
    const int slotNumber = slotId.mid(5).toInt();
    return QStringLiteral("Shortcuts/slot%1/%2").arg(slotNumber).arg(name);
}

bool isNoiseMode(int value)
{
    switch (static_cast<ShortcutActionManager::NoiseMode>(value)) {
    case ShortcutActionManager::NoiseMode::Off:
    case ShortcutActionManager::NoiseMode::Transparency:
    case ShortcutActionManager::NoiseMode::Adaptive:
    case ShortcutActionManager::NoiseMode::WindCancellation:
    case ShortcutActionManager::NoiseMode::NoiseCancellation:
        return true;
    case ShortcutActionManager::NoiseMode::None:
        return false;
    }

    return false;
}

QVariantMap actionItem(const QString &text,
                       ShortcutActionManager::Action action,
                       const QVariantList &params = {})
{
    return {
        {QStringLiteral("text"), text},
        {QStringLiteral("action"), static_cast<int>(action)},
        {QStringLiteral("params"), params},
    };
}

bool isDeviceAction(ShortcutActionManager::Action action)
{
    return action == ShortcutActionManager::Action::ToggleDeviceConnection
        || action == ShortcutActionManager::Action::ConnectDevice
        || action == ShortcutActionManager::Action::DisconnectDevice;
}

void appendDeviceActions(QVariantList &actions,
                         const QString &name,
                         const QString &address)
{
    actions.append(actionItem(qtTrId("shortcuts.action.toggle_device_connection").arg(name),
                              ShortcutActionManager::Action::ToggleDeviceConnection,
                              {address}));
    actions.append(actionItem(qtTrId("shortcuts.action.connect_device").arg(name),
                              ShortcutActionManager::Action::ConnectDevice,
                              {address}));
    actions.append(actionItem(qtTrId("shortcuts.action.disconnect_device").arg(name),
                              ShortcutActionManager::Action::DisconnectDevice,
                              {address}));
}
}

ShortcutActionManager::ShortcutActionManager(Backend *backend,
                                             UiSettingsService *settings,
                                             std::function<void()> toggleWindow,
                                             QObject *parent)
    : QObject(parent)
    , backend(backend)
    , settings(settings)
    , toggleWindow(std::move(toggleWindow))
{
    Q_ASSERT(this->backend);
    Q_ASSERT(this->settings);

    connect(this->backend,
            &Backend::dataReceived,
            this,
            &ShortcutActionManager::handleDataReceived);
    connect(this->backend,
            &Backend::connectedChanged,
            this,
            &ShortcutActionManager::handleBackendConnectedChanged);

    handleBackendConnectedChanged();
}

ShortcutActionManager::Assignment ShortcutActionManager::assignment(
    const QString &slotId) const
{
    if (!isValidSlotId(slotId))
        return {};

    Assignment result;
    result.action = static_cast<Action>(
        settings->value(settingKey(slotId, QStringLiteral("action")), 0).toInt());
    const QStringList storedParams = settings->value(
        settingKey(slotId, QStringLiteral("params"))).toStringList();
    for (const QString &param : storedParams)
        result.params.append(param);

    if (result.action == Action::SetNoiseMode && result.params.size() == 1)
        result.params[0] = result.params[0].toInt();

    if (!isValidAssignment(result.action, result.params))
        return {};

    return result;
}

bool ShortcutActionManager::setAssignment(const QString &slotId,
                                          Action action,
                                          const QVariantList &params)
{
    if (!isValidSlotId(slotId) || !isValidAssignment(action, params))
        return false;

    if (action == Action::None)
        return clearAssignment(slotId);

    const QString paramsKey = settingKey(slotId, QStringLiteral("params"));
    if (params.isEmpty()) {
        if (!settings->remove(paramsKey))
            return false;
    } else {
        QStringList storedParams;
        for (const QVariant &param : params)
            storedParams.append(param.toString());

        if (!settings->setValue(paramsKey, storedParams))
            return false;
    }

    const bool saved = settings->setValue(
        settingKey(slotId, QStringLiteral("action")),
        static_cast<int>(action));
    if (saved)
        emit availableActionsChanged();

    return saved;
}

bool ShortcutActionManager::clearAssignment(const QString &slotId)
{
    const bool actionRemoved = settings->remove(
        settingKey(slotId, QStringLiteral("action")));
    const bool paramsRemoved = settings->remove(
        settingKey(slotId, QStringLiteral("params")));
    const bool removed = actionRemoved && paramsRemoved;
    if (removed)
        emit availableActionsChanged();

    return removed;
}

QVariantList ShortcutActionManager::availableActions() const
{
    QVariantList actions{
        actionItem(qtTrId("shortcuts.action.none"), Action::None),
        actionItem(qtTrId("shortcuts.action.toggle_window"), Action::ToggleWindow),
        actionItem(qtTrId("shortcuts.action.toggle_bluetooth"), Action::ToggleBluetooth),
        actionItem(qtTrId("shortcuts.action.enable_bluetooth"), Action::EnableBluetooth),
        actionItem(qtTrId("shortcuts.action.disable_bluetooth"), Action::DisableBluetooth),
        actionItem(qtTrId("shortcuts.action.cycle_noise_control"),
                   Action::CycleNoiseControl),
        actionItem(qtTrId("shortcuts.action.noise_cancellation"),
                   Action::SetNoiseMode,
                   {static_cast<int>(NoiseMode::NoiseCancellation)}),
        actionItem(qtTrId("shortcuts.action.adaptive"),
                   Action::SetNoiseMode,
                   {static_cast<int>(NoiseMode::Adaptive)}),
        actionItem(qtTrId("shortcuts.action.transparency"),
                   Action::SetNoiseMode,
                   {static_cast<int>(NoiseMode::Transparency)}),
        actionItem(qtTrId("shortcuts.action.wind_reduction"),
                   Action::SetNoiseMode,
                   {static_cast<int>(NoiseMode::WindCancellation)}),
        actionItem(qtTrId("shortcuts.action.noise_control_off"),
                   Action::SetNoiseMode,
                   {static_cast<int>(NoiseMode::Off)}),
    };

    QSet<QString> listedAddresses;
    for (const QVariant &headphoneValue : headphonesData) {
        const QVariantMap headphone = headphoneValue.toMap();
        const QString address = headphone.value(QStringLiteral("address")).toString();
        if (address.isEmpty())
            continue;

        QString name = headphone.value(QStringLiteral("name")).toString();
        if (name.isEmpty())
            name = address;

        listedAddresses.insert(address);
        appendDeviceActions(actions, name, address);
    }

    for (int slotNumber = 1; slotNumber <= kShortcutSlotCount; ++slotNumber) {
        const Assignment savedAssignment = assignment(shortcutId(slotNumber));
        if (!isDeviceAction(savedAssignment.action))
            continue;

        const QString address = savedAssignment.params[0].toString();
        if (listedAddresses.contains(address))
            continue;

        listedAddresses.insert(address);
        appendDeviceActions(actions, address, address);
    }

    return actions;
}

void ShortcutActionManager::refreshTranslations()
{
    emit availableActionsChanged();
}

int ShortcutActionManager::assignmentIndex(const QString &slotId) const
{
    const Assignment currentAssignment = assignment(slotId);
    const QVariantList actions = availableActions();
    for (int index = 0; index < actions.size(); ++index) {
        const QVariantMap item = actions[index].toMap();
        if (item.value(QStringLiteral("action")).toInt()
                == static_cast<int>(currentAssignment.action)
            && item.value(QStringLiteral("params")).toList()
                == currentAssignment.params) {
            return index;
        }
    }

    return 0;
}

bool ShortcutActionManager::setAssignmentFromIndex(const QString &slotId,
                                                   int index)
{
    const QVariantList actions = availableActions();
    if (index < 0 || index >= actions.size())
        return false;

    const QVariantMap item = actions[index].toMap();
    return setAssignment(
        slotId,
        static_cast<Action>(item.value(QStringLiteral("action")).toInt()),
        item.value(QStringLiteral("params")).toList());
}

void ShortcutActionManager::executeShortcut(const QString &slotId)
{
    const Assignment currentAssignment = assignment(slotId);
    auto deadline = activationDeadlines.find(slotId);
    if (deadline != activationDeadlines.end() && !deadline.value().hasExpired()) {
        deadline.value() = QDeadlineTimer(kActivationQuietPeriodMs);
        return;
    }

    activationDeadlines.insert(slotId, QDeadlineTimer(kActivationQuietPeriodMs));

    switch (currentAssignment.action) {
    case Action::None:
        return;
    case Action::ToggleWindow:
        if (toggleWindow)
            toggleWindow();
        return;
    case Action::ToggleBluetooth:
        toggleBluetooth();
        return;
    case Action::EnableBluetooth:
        setBluetoothEnabled(true);
        return;
    case Action::DisableBluetooth:
        setBluetoothEnabled(false);
        return;
    case Action::CycleNoiseControl:
        cycleNoiseControl();
        return;
    case Action::SetNoiseMode:
        setNoiseMode(currentAssignment.params);
        return;
    case Action::ToggleDeviceConnection:
        toggleDeviceConnection(currentAssignment.params);
        return;
    case Action::ConnectDevice:
        setDeviceConnected(currentAssignment.params, true);
        return;
    case Action::DisconnectDevice:
        setDeviceConnected(currentAssignment.params, false);
        return;
    }
}

bool ShortcutActionManager::isValidSlotId(const QString &slotId) const
{
    for (int slotNumber = 1; slotNumber <= kShortcutSlotCount; ++slotNumber) {
        if (slotId == shortcutId(slotNumber))
            return true;
    }

    return false;
}

bool ShortcutActionManager::isValidAssignment(Action action,
                                              const QVariantList &params) const
{
    switch (action) {
    case Action::None:
    case Action::ToggleWindow:
    case Action::ToggleBluetooth:
    case Action::EnableBluetooth:
    case Action::DisableBluetooth:
    case Action::CycleNoiseControl:
        // Actions without parameters are invalid if params contains a value.
        return params.isEmpty();
    case Action::SetNoiseMode:
        return params.size() == 1
            && params[0].canConvert<int>()
            && isNoiseMode(params[0].toInt());
    case Action::ToggleDeviceConnection:
    case Action::ConnectDevice:
    case Action::DisconnectDevice:
        return params.size() == 1
            && !params[0].toString().isEmpty();
    }

    return false;
}

void ShortcutActionManager::handleDataReceived(const QVariant &json)
{
    const QVariantMap data = json.toMap();
    if (data.isEmpty()) {
        const bool actionsChanged = !headphonesData.isEmpty();
        bluetoothAdapterData.clear();
        headphonesData.clear();
        infoData.clear();
        if (actionsChanged)
            emit availableActionsChanged();
        return;
    }

    if (data.contains(QStringLiteral("defaultbluetooth"))) {
        bluetoothAdapterData = data.value(QStringLiteral("defaultbluetooth")).toMap();
    }
    if (data.contains(QStringLiteral("headphones"))) {
        headphonesData = data.value(QStringLiteral("headphones")).toList();
        emit availableActionsChanged();
    }
    if (data.contains(QStringLiteral("info"))) {
        infoData = data.value(QStringLiteral("info")).toMap();
    }
}

void ShortcutActionManager::handleBackendConnectedChanged()
{
    if (!backend->connected()) {
        bluetoothAdapterData.clear();
        infoData.clear();
        return;
    }

    backend->getAll();
}

void ShortcutActionManager::toggleBluetooth()
{
    if (bluetoothAdapterData.isEmpty())
        return;

    setBluetoothEnabled(!bluetoothAdapterData.value(
                             QStringLiteral("enabled")).toBool());
}

void ShortcutActionManager::setBluetoothEnabled(bool enabled)
{
    if (!backend->connected())
        return;

    if (enabled)
        backend->enableDefaultBluetoothAdapter();
    else
        backend->disableDefaultBluetoothAdapter();

    bluetoothAdapterData.insert(QStringLiteral("enabled"), enabled);
}

void ShortcutActionManager::cycleNoiseControl()
{
    const QString address = infoData.value(QStringLiteral("address")).toString();
    const QVariantMap ancData = ancDataForAddress(address);
    if (ancData.isEmpty() || ancData.value(QStringLiteral("readonly")).toBool())
        return;

    const int options = ancData.value(QStringLiteral("options")).toInt();
    const int selected = ancData.value(QStringLiteral("selected")).toInt();
    constexpr std::array<int, 5> modes{
        static_cast<int>(NoiseMode::Off),
        static_cast<int>(NoiseMode::Transparency),
        static_cast<int>(NoiseMode::Adaptive),
        static_cast<int>(NoiseMode::WindCancellation),
        static_cast<int>(NoiseMode::NoiseCancellation),
    };

    int selectedIndex = -1;
    for (int index = 0; index < static_cast<int>(modes.size()); ++index) {
        if (modes[index] == selected) {
            selectedIndex = index;
            break;
        }
    }

    for (int offset = 1; offset <= static_cast<int>(modes.size()); ++offset) {
        const int index = (selectedIndex + offset) % static_cast<int>(modes.size());
        const int mode = modes[index];
        if ((options & mode) == 0 || mode == selected)
            continue;

        backend->setAnc(address, mode);
        updateSelectedNoiseMode(mode);
        return;
    }
}

void ShortcutActionManager::setNoiseMode(const QVariantList &params)
{
    const QString address = infoData.value(QStringLiteral("address")).toString();
    const int mode = params[0].toInt();
    const QVariantMap ancData = ancDataForAddress(address);
    if (ancData.isEmpty()
        || ancData.value(QStringLiteral("readonly")).toBool()
        || (ancData.value(QStringLiteral("options")).toInt() & mode) == 0) {
        return;
    }

    backend->setAnc(address, mode);
    updateSelectedNoiseMode(mode);
}

void ShortcutActionManager::toggleDeviceConnection(const QVariantList &params)
{
    const QString address = params[0].toString();
    const QVariantMap headphone = headphoneForAddress(address);
    if (headphone.isEmpty())
        return;

    setDeviceConnected(params,
                       !headphone.value(QStringLiteral("connected")).toBool());
}

void ShortcutActionManager::setDeviceConnected(const QVariantList &params,
                                               bool connected)
{
    if (!backend->connected())
        return;

    const QString address = params[0].toString();
    if (connected)
        backend->connectDevice(address);
    else
        backend->disconnectDevice(address);

    updateHeadphoneConnected(address, connected);
}

QVariantMap ShortcutActionManager::ancDataForAddress(const QString &address) const
{
    if (address.isEmpty()
        || infoData.value(QStringLiteral("address")).toString() != address) {
        return {};
    }

    return infoData.value(QStringLiteral("capabilities"))
        .toMap()
        .value(QStringLiteral("anc"))
        .toMap();
}

QVariantMap ShortcutActionManager::headphoneForAddress(const QString &address) const
{
    for (const QVariant &headphoneValue : headphonesData) {
        const QVariantMap headphone = headphoneValue.toMap();
        if (headphone.value(QStringLiteral("address")).toString() == address)
            return headphone;
    }

    return {};
}

void ShortcutActionManager::updateHeadphoneConnected(const QString &address,
                                                     bool connected)
{
    for (int index = 0; index < headphonesData.size(); ++index) {
        QVariantMap headphone = headphonesData[index].toMap();
        if (headphone.value(QStringLiteral("address")).toString() != address)
            continue;

        headphone.insert(QStringLiteral("connected"), connected);
        headphonesData[index] = headphone;
        return;
    }
}

void ShortcutActionManager::updateSelectedNoiseMode(int mode)
{
    QVariantMap capabilities = infoData.value(
        QStringLiteral("capabilities")).toMap();
    QVariantMap ancData = capabilities.value(QStringLiteral("anc")).toMap();
    ancData.insert(QStringLiteral("selected"), mode);
    capabilities.insert(QStringLiteral("anc"), ancData);
    infoData.insert(QStringLiteral("capabilities"), capabilities);
}
