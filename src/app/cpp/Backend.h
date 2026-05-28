// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <QObject>
#include <QTimer>
#include <QVariant>
#include <QWebSocket>

class Backend final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool unsupportedApi READ unsupportedApi NOTIFY unsupportedApiChanged)
    Q_PROPERTY(QString backendInfoText READ backendInfoText NOTIFY backendInfoChanged)

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend() override;

    bool connected() const;
    bool unsupportedApi() const;
    QString backendInfoText() const;

    Q_INVOKABLE void connectSocket();
    Q_INVOKABLE void disconnectSocket();
    Q_INVOKABLE void getAll();
    Q_INVOKABLE void getInfo();
    Q_INVOKABLE void getDevices();
    Q_INVOKABLE void getDefaultBluetoothAdapter();
    Q_INVOKABLE void getSetting(const QString &containerName, const QString &settingName);
    Q_INVOKABLE void setSetting(const QString &containerName, const QString &settingName, const QVariant &newValue);
    Q_INVOKABLE void connectDevice(const QString &address);
    Q_INVOKABLE void disconnectDevice(const QString &address);
    Q_INVOKABLE void enableDefaultBluetoothAdapter();
    Q_INVOKABLE void disableDefaultBluetoothAdapter();
    Q_INVOKABLE void setAnc(const QString &address, int value);
    Q_INVOKABLE void setCapability(const QString &capability, const QString &address, const QVariant &value);
    Q_INVOKABLE void setLogLevel(int value);

signals:
    void dataReceived(const QVariant &json);
    void connectedChanged();
    void unsupportedApiChanged();
    void backendInfoChanged();

private:
    void resetSessionState();
    void restartTransport();
    void sendToSocket(const QByteArray &payload);
    void sendJson(const QVariantMap &payload);
    void updateApiReady(bool ready);
    void updateBackendInfoText(const QString &text);
    void updateUnsupportedApi(bool unsupported);
    void notifyConnectedChanged();
    void scheduleReconnect();
    void handleTextMessageReceived(const QString &message);

    QWebSocket socket;
    QTimer reconnectTimer;
    bool allowReconnect = true;
    int supportedApiVersion = 0;
    int maxAttempts = -1;
    int reconnectAttempts = 0;
    QString backendInfoTextValue;
    bool apiReady = false;
    bool unsupportedApiValue = false;
};
