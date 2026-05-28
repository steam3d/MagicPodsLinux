// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "Backend.h"

#include <QDebug>
#include <QMetaType>

#include <nlohmann/json.hpp>

namespace {
using json = nlohmann::json;

QVariant jsonToVariant(const json &value)
{
    if (value.is_null()) {
        return {};
    }
    if (value.is_boolean()) {
        return value.get<bool>();
    }
    if (value.is_number_integer()) {
        return static_cast<qint64>(value.get<json::number_integer_t>());
    }
    if (value.is_number_unsigned()) {
        return static_cast<quint64>(value.get<json::number_unsigned_t>());
    }
    if (value.is_number_float()) {
        return value.get<double>();
    }
    if (value.is_string()) {
        return QString::fromStdString(value.get<std::string>());
    }
    if (value.is_array()) {
        QVariantList list;
        list.reserve(static_cast<qsizetype>(value.size()));
        for (const auto &item : value) {
            list.push_back(jsonToVariant(item));
        }
        return list;
    }
    if (value.is_object()) {
        QVariantMap map;
        for (auto it = value.begin(); it != value.end(); ++it) {
            map.insert(QString::fromStdString(it.key()), jsonToVariant(it.value()));
        }
        return map;
    }
    return {};
}

json variantToJson(const QVariant &value)
{
    if (!value.isValid() || value.isNull()) {
        return nullptr;
    }

    switch (value.typeId()) {
    case QMetaType::Bool:
        return value.toBool();
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::LongLong:
        return value.toLongLong();
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULongLong:
        return value.toULongLong();
    case QMetaType::Float:
    case QMetaType::Double:
        return value.toDouble();
    case QMetaType::QString:
        return value.toString().toStdString();
    case QMetaType::QStringList: {
        json array = json::array();
        for (const auto &item : value.toStringList()) {
            array.push_back(item.toStdString());
        }
        return array;
    }
    default:
        break;
    }

    if (value.canConvert<QVariantMap>()) {
        const QVariantMap map = value.toMap();
        json object = json::object();
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            object[it.key().toStdString()] = variantToJson(it.value());
        }
        return object;
    }

    if (value.canConvert<QVariantList>()) {
        const QVariantList list = value.toList();
        json array = json::array();
        for (const auto &item : list) {
            array.push_back(variantToJson(item));
        }
        return array;
    }

    return value.toString().toStdString();
}
}

Backend::Backend(QObject *parent)
    : QObject(parent)
{
    reconnectTimer.setInterval(1000);
    reconnectTimer.setSingleShot(true);

    connect(&socket, &QWebSocket::connected, this, [this]() {
        reconnectTimer.stop();
        notifyConnectedChanged();
    });
    connect(&socket, &QWebSocket::disconnected, this, [this]() {
        resetSessionState();
        notifyConnectedChanged();
        scheduleReconnect();
    });
    connect(&socket, &QWebSocket::stateChanged, this, [this]() {
        notifyConnectedChanged();
    });
    connect(&socket, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        resetSessionState();
        notifyConnectedChanged();
        scheduleReconnect();
    });
    connect(&socket, &QWebSocket::textMessageReceived, this, &Backend::handleTextMessageReceived);
    connect(&reconnectTimer, &QTimer::timeout, this, [this]() {
        if (!allowReconnect) {
            return;
        }
        restartTransport();
    });
}

Backend::~Backend()
{
    disconnectSocket();
}

bool Backend::connected() const
{
    return socket.state() == QAbstractSocket::ConnectedState && apiReady;
}

bool Backend::unsupportedApi() const
{
    return unsupportedApiValue;
}

QString Backend::backendInfoText() const
{
    return backendInfoTextValue;
}

void Backend::connectSocket()
{
    reconnectAttempts = maxAttempts;
    allowReconnect = true;

    resetSessionState();
    restartTransport();
}

void Backend::disconnectSocket()
{
    allowReconnect = false;
    reconnectTimer.stop();
    if (socket.state() != QAbstractSocket::UnconnectedState) {
        socket.abort();
        socket.close();
    }
    resetSessionState();
    notifyConnectedChanged();
}

void Backend::getAll()
{
    sendJson({{QStringLiteral("method"), QStringLiteral("GetAll")}});
}

void Backend::getInfo()
{
    sendJson({{QStringLiteral("method"), QStringLiteral("GetActiveDeviceInfo")}});
}

void Backend::getDevices()
{
    sendJson({{QStringLiteral("method"), QStringLiteral("GetDevices")}});
}

void Backend::getDefaultBluetoothAdapter()
{
    sendJson({{QStringLiteral("method"), QStringLiteral("GetDefaultBluetoothAdapter")}});
}

void Backend::getSetting(const QString &containerName, const QString &settingName)
{
    sendJson({
        {QStringLiteral("method"), QStringLiteral("GetSetting")},
        {QStringLiteral("arguments"), QVariantMap{
             {QStringLiteral("container"), containerName},
             {QStringLiteral("setting"), settingName},
         }},
    });
}

void Backend::setSetting(const QString &containerName, const QString &settingName, const QVariant &newValue)
{
    sendJson({
        {QStringLiteral("method"), QStringLiteral("SetSetting")},
        {QStringLiteral("arguments"), QVariantMap{
             {QStringLiteral("container"), containerName},
             {QStringLiteral("setting"), settingName},
             {QStringLiteral("value"), newValue},
         }},
    });
}

void Backend::connectDevice(const QString &address)
{
    sendJson({
        {QStringLiteral("method"), QStringLiteral("ConnectDevice")},
        {QStringLiteral("arguments"), QVariantMap{
             {QStringLiteral("address"), address},
         }},
    });
}

void Backend::disconnectDevice(const QString &address)
{
    sendJson({
        {QStringLiteral("method"), QStringLiteral("DisconnectDevice")},
        {QStringLiteral("arguments"), QVariantMap{
             {QStringLiteral("address"), address},
         }},
    });
}

void Backend::enableDefaultBluetoothAdapter()
{
    sendJson({{QStringLiteral("method"), QStringLiteral("EnableDefaultBluetoothAdapter")}});
}

void Backend::disableDefaultBluetoothAdapter()
{
    sendJson({{QStringLiteral("method"), QStringLiteral("DisableDefaultBluetoothAdapter")}});
}

void Backend::setAnc(const QString &address, int value)
{
    sendJson({
        {QStringLiteral("method"), QStringLiteral("SetCapabilities")},
        {QStringLiteral("arguments"), QVariantMap{
             {QStringLiteral("address"), address},
             {QStringLiteral("capabilities"), QVariantMap{
                  {QStringLiteral("anc"), QVariantMap{
                       {QStringLiteral("selected"), value},
                   }},
              }},
         }},
    });
}

void Backend::setCapability(const QString &capability, const QString &address, const QVariant &value)
{
    QVariantMap capabilities;
    capabilities.insert(capability, QVariantMap{
                                       {QStringLiteral("selected"), value},
                                   });

    sendJson({
        {QStringLiteral("method"), QStringLiteral("SetCapabilities")},
        {QStringLiteral("arguments"), QVariantMap{
             {QStringLiteral("address"), address},
             {QStringLiteral("capabilities"), capabilities},
         }},
    });
}

void Backend::setLogLevel(int value)
{
    sendJson({
        {QStringLiteral("method"), QStringLiteral("SetLogLevel")},
        {QStringLiteral("arguments"), QVariantMap{
             {QStringLiteral("selected"), value},
         }},
    });
}

void Backend::resetSessionState()
{
    updateUnsupportedApi(false);
    updateBackendInfoText(QString{});
    updateApiReady(false);
}

void Backend::restartTransport()
{
    if (socket.state() != QAbstractSocket::UnconnectedState) {
        socket.abort();
        socket.close();
    }
    notifyConnectedChanged();
    socket.open(QUrl(QStringLiteral("ws://localhost:2020")));
}

void Backend::sendToSocket(const QByteArray &payload)
{
    if (socket.state() == QAbstractSocket::ConnectedState) {
        socket.sendTextMessage(QString::fromUtf8(payload));
    }
}

void Backend::sendJson(const QVariantMap &payload)
{
    sendToSocket(QByteArray::fromStdString(variantToJson(payload).dump()));
}

void Backend::updateApiReady(bool ready)
{
    if (apiReady == ready) {
        return;
    }
    const bool wasConnected = connected();
    apiReady = ready;
    if (wasConnected != connected()) {
        emit connectedChanged();
    }
}

void Backend::updateBackendInfoText(const QString &text)
{
    if (backendInfoTextValue == text) {
        return;
    }

    backendInfoTextValue = text;
    emit backendInfoChanged();
}

void Backend::updateUnsupportedApi(bool unsupported)
{
    if (unsupportedApiValue == unsupported) {
        return;
    }
    unsupportedApiValue = unsupported;
    emit unsupportedApiChanged();
}

void Backend::notifyConnectedChanged()
{
    emit connectedChanged();
}

void Backend::scheduleReconnect()
{
    if (!allowReconnect) {
        return;
    }
    if (reconnectAttempts == 0) {
        return;
    }
    if (maxAttempts >= 0) {
        reconnectAttempts -= 1;
    }
    reconnectTimer.start();
}

void Backend::handleTextMessageReceived(const QString &message)
{
    try {
        const json parsed = json::parse(message.toStdString());

        if (!apiReady) {
            const auto initIt = parsed.find("init");
            if (initIt == parsed.end() || !initIt->is_object()) {
                return;
            }

            const auto apiIt = initIt->find("api");
            if (apiIt == initIt->end()) {
                return;
            }

            const auto versionIt = initIt->find("version");
            const QString backendVersion = (versionIt != initIt->end() && versionIt->is_string())
                ? QString::fromStdString(versionIt->get<std::string>())
                : QString{};
            const int backendApiVersion = apiIt->get<int>();

            updateBackendInfoText(
                backendVersion.isEmpty()
                    ? QStringLiteral("API %1").arg(backendApiVersion)
                    : QStringLiteral("%1 (API %2)").arg(backendVersion).arg(backendApiVersion));

            if (backendApiVersion != supportedApiVersion) {
                updateUnsupportedApi(true);
                return;
            }
            updateApiReady(true);
        }

        emit dataReceived(jsonToVariant(parsed));
    } catch (const std::exception &e) {
        qWarning() << "Backend parse error" << e.what();
    }
}
