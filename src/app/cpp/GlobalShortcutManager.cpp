// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "GlobalShortcutManager.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QDebug>
#include <QUuid>

namespace {
constexpr auto kPortalService = "org.freedesktop.portal.Desktop";
constexpr auto kPortalPath = "/org/freedesktop/portal/desktop";
constexpr auto kGlobalShortcutsInterface = "org.freedesktop.portal.GlobalShortcuts";
constexpr auto kHostRegistryInterface = "org.freedesktop.host.portal.Registry";
constexpr auto kRequestInterface = "org.freedesktop.portal.Request";
constexpr auto kSessionInterface = "org.freedesktop.portal.Session";
constexpr auto kApplicationId = "app.magicpods"; //must be the same in the shortcut
constexpr int kShortcutSlotCount = 10;

QString shortcutId(int slotNumber)
{
    return QStringLiteral("slot-%1").arg(slotNumber, 2, 10, QLatin1Char('0'));
}

bool isShortcutId(const QString &id)
{
    for (int slotNumber = 1; slotNumber <= kShortcutSlotCount; ++slotNumber) {
        if (id == shortcutId(slotNumber))
            return true;
    }

    return false;
}

QVariant unpackVariant(const QVariant &value)
{
    if (value.metaType() == QMetaType::fromType<QDBusVariant>())
        return value.value<QDBusVariant>().variant();

    return value;
}

bool hasAssignedShortcut(const PortalShortcutList &shortcuts)
{
    for (const PortalShortcut &shortcut : shortcuts) {
        if (!isShortcutId(shortcut.id))
            continue;

        const QVariant triggerDescription = unpackVariant(
            shortcut.properties.value(QStringLiteral("trigger_description")));
        if (!triggerDescription.toString().trimmed().isEmpty())
            return true;
    }

    return false;
}

bool hasRegisteredShortcut(const PortalShortcutList &shortcuts)
{
    for (const PortalShortcut &shortcut : shortcuts) {
        if (isShortcutId(shortcut.id))
            return true;
    }

    return false;
}

PortalShortcutList decodeShortcuts(const QVariant &value)
{
    const QVariant unpacked = unpackVariant(value);
    if (unpacked.metaType() == QMetaType::fromType<QDBusArgument>())
        return qdbus_cast<PortalShortcutList>(unpacked.value<QDBusArgument>());

    return unpacked.value<PortalShortcutList>();
}
}

QDBusArgument &operator<<(QDBusArgument &argument, const PortalShortcut &shortcut)
{
    argument.beginStructure();
    argument << shortcut.id << shortcut.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, PortalShortcut &shortcut)
{
    argument.beginStructure();
    argument >> shortcut.id >> shortcut.properties;
    argument.endStructure();
    return argument;
}

GlobalShortcutManager::GlobalShortcutManager(bool gameScopeMode,
                                             bool desktopEntryAvailableAtStartup,
                                             QObject *parent)
    : QObject(parent)
    , gameScopeMode(gameScopeMode)
    , desktopEntryAvailableAtStartup(desktopEntryAvailableAtStartup)
{
    qRegisterMetaType<PortalShortcut>();
    qRegisterMetaType<PortalShortcutList>();
    qDBusRegisterMetaType<PortalShortcut>();
    qDBusRegisterMetaType<PortalShortcutList>();
}

GlobalShortcutManager::~GlobalShortcutManager()
{
    stop();
}

bool GlobalShortcutManager::available() const
{
    return availableValue;
}

bool GlobalShortcutManager::busy() const
{
    return busyValue;
}

bool GlobalShortcutManager::canConfigure() const
{
    return !sessionPath.isEmpty();
}

bool GlobalShortcutManager::desktopEntryAvailable() const
{
    return desktopEntryAvailableAtStartup;
}

void GlobalShortcutManager::start()
{
    if (gameScopeMode)
        return;

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qWarning() << "[GlobalShortcutManager] Session D-Bus is not available";
        return;
    }

    if (desktopEntryAvailableAtStartup)
        registerHostApplication(bus);

    QDBusInterface portal(QString::fromLatin1(kPortalService),
                          QString::fromLatin1(kPortalPath),
                          QString::fromLatin1(kGlobalShortcutsInterface),
                          bus);
    if (!portal.isValid()) {
        qWarning() << "[GlobalShortcutManager] GlobalShortcuts portal is not available";
        return;
    }

    portalVersion = portal.property("version").toUInt();
    if (portalVersion < 1) {
        qWarning() << "[GlobalShortcutManager] GlobalShortcuts portal v1 is required";
        return;
    }

    setAvailable(true);
    qInfo() << "[GlobalShortcutManager] Portal version" << portalVersion
            << "is available";

    if (!desktopEntryAvailableAtStartup)
        return;

    createSession();
}

void GlobalShortcutManager::stop()
{
    clearPendingRequest();
    closeSession();
    setBusy(false);
}

void GlobalShortcutManager::configureShortcuts()
{
    if (!canConfigure() || busyValue)
        return;

    if (!bindCompletedForSession) {
        if (bindAttemptedForSession) {
            closeSession();
            bindAfterSessionCreation = true;
            createSession();
        } else {
            bindShortcuts();
        }
        return;
    }

    if (portalVersion < 2) {
        emit configurationHelpRequested();
        return;
    }

    setBusy(true);

    QDBusInterface portal(QString::fromLatin1(kPortalService),
                          QString::fromLatin1(kPortalPath),
                          QString::fromLatin1(kGlobalShortcutsInterface),
                          QDBusConnection::sessionBus());
    const QDBusPendingCall call = portal.asyncCall(
        QStringLiteral("ConfigureShortcuts"),
        QVariant::fromValue(QDBusObjectPath(sessionPath)),
        QString(),
        QVariantMap());

    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *finishedWatcher) {
        QDBusPendingReply<> reply = *finishedWatcher;
        finishedWatcher->deleteLater();
        setBusy(false);

        if (reply.isError()) {
            const QString message = QStringLiteral("ConfigureShortcuts: %1: %2")
                                        .arg(reply.error().name(),
                                             reply.error().message());
            qWarning() << "[GlobalShortcutManager]" << message;
            return;
        }
    });
}

void GlobalShortcutManager::handleRequestResponse(uint response,
                                                  const QVariantMap &results)
{
    const RequestOperation operation = pendingOperation;
    clearPendingRequest();

    if (response != 0) {
        const QString reason = response == 1
            ? QStringLiteral("cancelled by the user")
            : QStringLiteral("portal error %1").arg(response);
        failRequest(QStringLiteral("Global shortcut request was %1").arg(reason));
        return;
    }

    if (operation == RequestOperation::CreateSession) {
        const QVariant rawHandle = unpackVariant(results.value(QStringLiteral("session_handle")));
        if (rawHandle.metaType() == QMetaType::fromType<QDBusObjectPath>())
            sessionPath = rawHandle.value<QDBusObjectPath>().path();
        else
            sessionPath = rawHandle.toString();

        if (sessionPath.isEmpty()) {
            failRequest(QStringLiteral("CreateSession returned an empty session handle"));
            return;
        }

        bindAttemptedForSession = false;
        bindCompletedForSession = false;
        emit canConfigureChanged();
        connectSessionSignals();
        if (bindAfterSessionCreation) {
            bindAfterSessionCreation = false;
            bindShortcuts();
        } else {
            listShortcuts();
        }
        return;
    }

    if (operation == RequestOperation::ListShortcuts) {
        const PortalShortcutList shortcuts = decodeShortcuts(
            results.value(QStringLiteral("shortcuts")));
        const bool shouldRestore = portalVersion < 2
            ? hasRegisteredShortcut(shortcuts)
            : hasAssignedShortcut(shortcuts);
        if (shouldRestore)
            bindShortcuts();
        else
            setBusy(false);
        return;
    }

    if (operation == RequestOperation::BindShortcuts) {
        bindCompletedForSession = true;
        setBusy(false);
        qInfo() << "[GlobalShortcutManager] Shortcut binding completed";
    }
}

void GlobalShortcutManager::handleActivated(const QDBusObjectPath &sessionHandle,
                                            const QString &shortcutId,
                                            qulonglong timestamp,
                                            const QVariantMap &options)
{
    Q_UNUSED(timestamp)
    Q_UNUSED(options)

    if (sessionHandle.path() != sessionPath || !isShortcutId(shortcutId)) {
        return;
    }

    qInfo() << "[GlobalShortcutManager] Activated" << shortcutId;
    emit shortcutActivated(shortcutId);
}

void GlobalShortcutManager::handleSessionClosed()
{
    disconnectSessionSignals();
    sessionPath.clear();
    bindAttemptedForSession = false;
    bindCompletedForSession = false;
    setBusy(false);
    emit canConfigureChanged();
    qInfo() << "[GlobalShortcutManager] Session closed";
}

QString GlobalShortcutManager::createToken(const QString &prefix) const
{
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    uuid.remove(QLatin1Char('-'));
    return prefix + uuid;
}

QString GlobalShortcutManager::expectedRequestPath(const QString &token) const
{
    QString sender = QDBusConnection::sessionBus().baseService();
    if (sender.startsWith(QLatin1Char(':')))
        sender.remove(0, 1);
    sender.replace(QLatin1Char('.'), QLatin1Char('_'));
    return QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
        .arg(sender, token);
}

void GlobalShortcutManager::registerHostApplication(const QDBusConnection &bus)
{
    QDBusMessage request = QDBusMessage::createMethodCall(
        QString::fromLatin1(kPortalService),
        QString::fromLatin1(kPortalPath),
        QString::fromLatin1(kHostRegistryInterface),
        QStringLiteral("Register"));
    request << QString::fromLatin1(kApplicationId)
            << QVariant::fromValue(QVariantMap());

    const QDBusMessage reply = bus.call(request);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "[GlobalShortcutManager] Host app registration failed:"
                   << reply.errorName() << reply.errorMessage();
        return;
    }

    qInfo() << "[GlobalShortcutManager] Registered host app ID"
            << kApplicationId;
}

bool GlobalShortcutManager::beginRequest(RequestOperation operation,
                                         const QString &token)
{
    clearPendingRequest();
    pendingOperation = operation;
    pendingRequestToken = token;
    pendingRequestPath = expectedRequestPath(token);

    const bool connected = QDBusConnection::sessionBus().connect(
        QString::fromLatin1(kPortalService),
        pendingRequestPath,
        QString::fromLatin1(kRequestInterface),
        QStringLiteral("Response"),
        this,
        SLOT(handleRequestResponse(uint,QVariantMap)));
    if (!connected) {
        failRequest(QStringLiteral("Cannot subscribe to the portal request response"));
        return false;
    }

    return true;
}

void GlobalShortcutManager::watchRequestCall(const QDBusPendingCall &call,
                                             const QString &token)
{
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, token](QDBusPendingCallWatcher *finishedWatcher) {
        QDBusPendingReply<QDBusObjectPath> reply = *finishedWatcher;
        finishedWatcher->deleteLater();

        if (pendingRequestToken != token)
            return;

        if (reply.isError()) {
            failRequest(QStringLiteral("Portal method: %1: %2")
                            .arg(reply.error().name(), reply.error().message()));
            return;
        }

        const QString returnedPath = reply.value().path();
        if (returnedPath.isEmpty() || returnedPath == pendingRequestPath)
            return;

        QDBusConnection::sessionBus().disconnect(
            QString::fromLatin1(kPortalService),
            pendingRequestPath,
            QString::fromLatin1(kRequestInterface),
            QStringLiteral("Response"),
            this,
            SLOT(handleRequestResponse(uint,QVariantMap)));
        pendingRequestPath = returnedPath;

        if (!QDBusConnection::sessionBus().connect(
                QString::fromLatin1(kPortalService),
                pendingRequestPath,
                QString::fromLatin1(kRequestInterface),
                QStringLiteral("Response"),
                this,
                SLOT(handleRequestResponse(uint,QVariantMap)))) {
            failRequest(QStringLiteral("Cannot subscribe to the returned portal request"));
        }
    });
}

void GlobalShortcutManager::clearPendingRequest()
{
    if (!pendingRequestPath.isEmpty()) {
        QDBusConnection::sessionBus().disconnect(
            QString::fromLatin1(kPortalService),
            pendingRequestPath,
            QString::fromLatin1(kRequestInterface),
            QStringLiteral("Response"),
            this,
            SLOT(handleRequestResponse(uint,QVariantMap)));
    }

    pendingRequestPath.clear();
    pendingRequestToken.clear();
    pendingOperation = RequestOperation::None;
}

void GlobalShortcutManager::createSession()
{
    const QString handleToken = createToken(QStringLiteral("magicpods_request_"));
    const QString sessionToken = createToken(QStringLiteral("magicpods_session_"));
    if (!beginRequest(RequestOperation::CreateSession, handleToken))
        return;

    setBusy(true);

    QVariantMap options;
    options.insert(QStringLiteral("handle_token"), handleToken);
    options.insert(QStringLiteral("session_handle_token"), sessionToken);

    QDBusInterface portal(QString::fromLatin1(kPortalService),
                          QString::fromLatin1(kPortalPath),
                          QString::fromLatin1(kGlobalShortcutsInterface),
                          QDBusConnection::sessionBus());
    watchRequestCall(portal.asyncCall(QStringLiteral("CreateSession"), options),
                     handleToken);
}

void GlobalShortcutManager::listShortcuts()
{
    const QString handleToken = createToken(QStringLiteral("magicpods_list_"));
    if (!beginRequest(RequestOperation::ListShortcuts, handleToken))
        return;

    setBusy(true);

    QVariantMap options;
    options.insert(QStringLiteral("handle_token"), handleToken);

    QVariantList arguments;
    arguments.append(QVariant::fromValue(QDBusObjectPath(sessionPath)));
    arguments.append(options);

    QDBusInterface portal(QString::fromLatin1(kPortalService),
                          QString::fromLatin1(kPortalPath),
                          QString::fromLatin1(kGlobalShortcutsInterface),
                          QDBusConnection::sessionBus());
    watchRequestCall(
        portal.asyncCallWithArgumentList(QStringLiteral("ListShortcuts"), arguments),
        handleToken);
}

void GlobalShortcutManager::bindShortcuts()
{
    const QString handleToken = createToken(QStringLiteral("magicpods_bind_"));
    if (!beginRequest(RequestOperation::BindShortcuts, handleToken))
        return;

    bindAttemptedForSession = true;
    setBusy(true);

    PortalShortcutList shortcuts;
    for (int slotNumber = 1; slotNumber <= kShortcutSlotCount; ++slotNumber) {
        shortcuts.append({
            shortcutId(slotNumber),
            {
                {QStringLiteral("description"),
                 QStringLiteral("Slot %1").arg(slotNumber)},
            },
        });
    }

    QVariantMap options;
    options.insert(QStringLiteral("handle_token"), handleToken);

    QVariantList arguments;
    arguments.append(QVariant::fromValue(QDBusObjectPath(sessionPath)));
    arguments.append(QVariant::fromValue(shortcuts));
    arguments.append(QString());
    arguments.append(options);

    QDBusInterface portal(QString::fromLatin1(kPortalService),
                          QString::fromLatin1(kPortalPath),
                          QString::fromLatin1(kGlobalShortcutsInterface),
                          QDBusConnection::sessionBus());
    watchRequestCall(
        portal.asyncCallWithArgumentList(QStringLiteral("BindShortcuts"), arguments),
        handleToken);
}

void GlobalShortcutManager::connectSessionSignals()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    const bool activatedConnected = bus.connect(
        QString::fromLatin1(kPortalService),
        QString::fromLatin1(kPortalPath),
        QString::fromLatin1(kGlobalShortcutsInterface),
        QStringLiteral("Activated"),
        this,
        SLOT(handleActivated(QDBusObjectPath,QString,qulonglong,QVariantMap)));
    const bool closedConnected = bus.connect(
        QString::fromLatin1(kPortalService),
        sessionPath,
        QString::fromLatin1(kSessionInterface),
        QStringLiteral("Closed"),
        this,
        SLOT(handleSessionClosed()));

    if (!activatedConnected || !closedConnected) {
        qWarning() << "[GlobalShortcutManager] Failed to connect one or more portal signals";
    }
}

void GlobalShortcutManager::disconnectSessionSignals()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.disconnect(
        QString::fromLatin1(kPortalService),
        QString::fromLatin1(kPortalPath),
        QString::fromLatin1(kGlobalShortcutsInterface),
        QStringLiteral("Activated"),
        this,
        SLOT(handleActivated(QDBusObjectPath,QString,qulonglong,QVariantMap)));
    bus.disconnect(
        QString::fromLatin1(kPortalService),
        sessionPath,
        QString::fromLatin1(kSessionInterface),
        QStringLiteral("Closed"),
        this,
        SLOT(handleSessionClosed()));
}

void GlobalShortcutManager::closeSession()
{
    if (sessionPath.isEmpty())
        return;

    disconnectSessionSignals();

    QDBusInterface session(QString::fromLatin1(kPortalService),
                           sessionPath,
                           QString::fromLatin1(kSessionInterface),
                           QDBusConnection::sessionBus());
    session.asyncCall(QStringLiteral("Close"));
    sessionPath.clear();
    bindAttemptedForSession = false;
    bindCompletedForSession = false;
    emit canConfigureChanged();
}

void GlobalShortcutManager::failRequest(const QString &message)
{
    clearPendingRequest();
    setBusy(false);
    qWarning() << "[GlobalShortcutManager]" << message;
}

void GlobalShortcutManager::setAvailable(bool value)
{
    if (availableValue == value)
        return;
    availableValue = value;
    emit availableChanged();
}

void GlobalShortcutManager::setBusy(bool value)
{
    if (busyValue == value)
        return;
    busyValue = value;
    emit busyChanged();
}
