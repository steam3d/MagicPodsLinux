// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <functional>

#include <QDeadlineTimer>
#include <QHash>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class Backend;
class UiSettingsService;

class ShortcutActionManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList availableActions READ availableActions NOTIFY availableActionsChanged)

public:
    // These values are stored in QSettings and must not be renumbered.
    enum class Action
    {
        None = 0,

        ToggleWindow = 1,

        ToggleBluetooth = 2,
        EnableBluetooth = 3,
        DisableBluetooth = 4,

        CycleNoiseControl = 5,
        SetNoiseMode = 6,

        ToggleDeviceConnection = 7,
        ConnectDevice = 8,
        DisconnectDevice = 9,
    };

    // These values match the backend ANC bit flags.
    enum class NoiseMode
    {
        None = 0,
        Off = 1,
        Transparency = 2,
        Adaptive = 4,
        WindCancellation = 8,
        NoiseCancellation = 16,
    };

    explicit ShortcutActionManager(Backend *backend,
                                   UiSettingsService *settings,
                                   std::function<void()> toggleWindow,
                                   QObject *parent = nullptr);

    QVariantList availableActions() const;
    Q_INVOKABLE int assignmentIndex(const QString &slotId) const;
    Q_INVOKABLE bool setAssignmentFromIndex(const QString &slotId, int index);

signals:
    void availableActionsChanged();

public slots:
    void refreshTranslations();
    void executeShortcut(const QString &slotId);

private:
    // Parameter formats:
    // SetNoiseMode: [mode]. Device actions: [address].
    struct Assignment
    {
        Action action = Action::None;
        QVariantList params;
    };

    Assignment assignment(const QString &slotId) const;
    bool setAssignment(const QString &slotId,
                       Action action,
                       const QVariantList &params);
    bool clearAssignment(const QString &slotId);
    bool isValidSlotId(const QString &slotId) const;
    bool isValidAssignment(Action action, const QVariantList &params) const;
    void handleDataReceived(const QVariant &json);
    void handleBackendConnectedChanged();
    void toggleBluetooth();
    void setBluetoothEnabled(bool enabled);
    void cycleNoiseControl();
    void setNoiseMode(const QVariantList &params);
    void toggleDeviceConnection(const QVariantList &params);
    void setDeviceConnected(const QVariantList &params, bool connected);
    QVariantMap ancDataForAddress(const QString &address) const;
    QVariantMap headphoneForAddress(const QString &address) const;
    void updateHeadphoneConnected(const QString &address, bool connected);
    void updateSelectedNoiseMode(int mode);

    Backend *backend = nullptr;
    UiSettingsService *settings = nullptr;
    std::function<void()> toggleWindow;
    QVariantMap bluetoothAdapterData;
    QVariantList headphonesData;
    QVariantMap infoData;
    QHash<QString, QDeadlineTimer> activationDeadlines;
};
