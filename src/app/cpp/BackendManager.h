// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <QObject>
#include <atomic>



class BackendManager : public QObject
{
    Q_OBJECT

public:
    explicit BackendManager(QObject *parent = nullptr);
    ~BackendManager() override;

    void beginShutdown();

    Q_INVOKABLE bool start();
    Q_INVOKABLE bool stop();
    Q_INVOKABLE bool restart();
    Q_INVOKABLE QString version();

    void recover();

private:
    QString binaryPath() const;
    bool isProcessAlive(qint64 pid) const;
    bool isRunning() const;
    void stopProcess();

    qint64 pid = 0;
    bool ownsBackend = false;
    std::atomic<bool> recovering{false};
    std::atomic<bool> shuttingDown{false};
};
