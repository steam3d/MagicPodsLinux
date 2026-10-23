// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

class QDBusConnection;

struct PortalShortcut
{
    QString id;
    QVariantMap properties;
};

using PortalShortcutList = QList<PortalShortcut>;

Q_DECLARE_METATYPE(PortalShortcut)
Q_DECLARE_METATYPE(PortalShortcutList)

QDBusArgument &operator<<(QDBusArgument &argument, const PortalShortcut &shortcut);
const QDBusArgument &operator>>(const QDBusArgument &argument, PortalShortcut &shortcut);

class GlobalShortcutManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool canConfigure READ canConfigure NOTIFY canConfigureChanged)
    Q_PROPERTY(bool desktopEntryAvailable READ desktopEntryAvailable CONSTANT)

public:
    explicit GlobalShortcutManager(bool gameScopeMode,
                                   bool desktopEntryAvailableAtStartup,
                                   QObject *parent = nullptr);
    ~GlobalShortcutManager() override;

    bool available() const;
    bool busy() const;
    bool canConfigure() const;
    bool desktopEntryAvailable() const;

    void start();
    void stop();

    Q_INVOKABLE void configureShortcuts();

signals:
    void availableChanged();
    void busyChanged();
    void canConfigureChanged();
    void shortcutActivated(const QString &shortcutId);

private slots:
    void handleRequestResponse(uint response, const QVariantMap &results);
    void handleActivated(const QDBusObjectPath &sessionHandle,
                         const QString &shortcutId,
                         qulonglong timestamp,
                         const QVariantMap &options);
    void handleShortcutsChanged(const QDBusObjectPath &sessionHandle,
                                const PortalShortcutList &shortcuts);
    void handleSessionClosed();

private:
    enum class RequestOperation
    {
        None,
        CreateSession,
        ListShortcuts,
        BindShortcuts,
    };

    QString createToken(const QString &prefix) const;
    QString expectedRequestPath(const QString &token) const;
    void registerHostApplication(const QDBusConnection &bus);
    bool beginRequest(RequestOperation operation, const QString &token);
    void watchRequestCall(const QDBusPendingCall &call, const QString &token);
    void clearPendingRequest();
    void createSession();
    void listShortcuts();
    void bindShortcuts();
    void connectSessionSignals();
    void disconnectSessionSignals();
    void closeSession();
    void applyShortcuts(const PortalShortcutList &shortcuts);
    void failRequest(const QString &message);
    void setAvailable(bool value);
    void setBusy(bool value);

    bool gameScopeMode = false;
    bool desktopEntryAvailableAtStartup = false;
    bool bindAfterSessionCreation = false;
    bool bindAttemptedForSession = false;
    bool availableValue = false;
    bool busyValue = false;
    bool shortcutsRegistered = false;
    QString sessionPath;
    QString pendingRequestPath;
    QString pendingRequestToken;
    RequestOperation pendingOperation = RequestOperation::None;
};
