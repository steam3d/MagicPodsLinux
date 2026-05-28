// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "BackendManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>

#include <csignal>

namespace {

QString normalizedPath(const QString &path)
{
    if (path.isEmpty())
        return {};

    const QFileInfo info(path);
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? QDir::cleanPath(info.absoluteFilePath()) : canonical;
}

void appendUniquePath(QStringList &paths, const QString &path)
{
    const QString normalized = normalizedPath(path);
    if (!normalized.isEmpty() && !paths.contains(normalized))
        paths.append(normalized);
}

QString backendLibraryPath()
{
    const QString configured = qEnvironmentVariable("MAGICPODSCORE_LIBDIR");
    if (configured.isEmpty())
        return {};

    const QFileInfo info(configured);
    if (!info.exists() || !info.isDir())
        return {};

    return normalizedPath(configured);
}

QProcessEnvironment backendProcessEnvironment()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString libPath = backendLibraryPath();
    if (libPath.isEmpty())
        return env;

    QStringList paths;
    appendUniquePath(paths, libPath);

    const QString inherited = env.value(QStringLiteral("LD_LIBRARY_PATH"));
    for (const QString &path : inherited.split(QLatin1Char(':'), Qt::SkipEmptyParts))
        appendUniquePath(paths, path);

    env.insert(QStringLiteral("LD_LIBRARY_PATH"), paths.join(QLatin1Char(':')));
    return env;
}

}



BackendManager::BackendManager(QObject *parent)
    : QObject(parent) {}

BackendManager::~BackendManager()
{
    beginShutdown();

    if (ownsBackend && pid > 0) {
        qDebug() << "[BackendManager] Terminating owned backend (pid" << pid << ") on exit";
        stopProcess();
    }
}



QString BackendManager::binaryPath() const
{
    QStringList candidates;

    const QString envOverride = qEnvironmentVariable("MAGICPODSCORE_PATH");
    appendUniquePath(candidates, envOverride);

    QStringList roots;
    appendUniquePath(roots, QCoreApplication::applicationDirPath());
    appendUniquePath(roots, QFileInfo(QCoreApplication::applicationFilePath()).absolutePath());
    appendUniquePath(roots, QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("..")));
    appendUniquePath(roots, qEnvironmentVariable("CQT_PKG_ROOT"));
    appendUniquePath(roots, qEnvironmentVariable("APPDIR"));
    appendUniquePath(roots, QDir::currentPath());

    for (const QString &root : std::as_const(roots)) {
        const QDir dir(root);
        appendUniquePath(candidates, dir.filePath(QStringLiteral("modules/magicpodscore")));
        appendUniquePath(candidates, dir.filePath(QStringLiteral("bin/modules/magicpodscore")));
        appendUniquePath(candidates, dir.filePath(QStringLiteral("../bin/modules/magicpodscore")));
    }

    appendUniquePath(candidates, QStandardPaths::findExecutable(QStringLiteral("magicpodscore")));

    for (const QString &path : std::as_const(candidates)) {
        const QFileInfo info(path);
        if (info.exists() && info.isFile() && info.isExecutable())
            return path;
    }

    return {};
}

bool BackendManager::isProcessAlive(qint64 pid) const
{
    if (pid <= 0) return false;
    return ::kill(static_cast<pid_t>(pid), 0) == 0;
}

bool BackendManager::isRunning() const
{
    if (ownsBackend && pid > 0)
        return isProcessAlive(pid);

    QProcess pgrep;
    pgrep.start(QStringLiteral("pgrep"), {QStringLiteral("-x"), QStringLiteral("magicpodscore")});
    if (!pgrep.waitForFinished(2000)) {
        pgrep.kill();
        return false;
    }
    return pgrep.exitCode() == 0;
}

void BackendManager::stopProcess()
{
    ::kill(static_cast<pid_t>(pid), SIGTERM);
    for (int i = 0; i < 30; ++i) {
        QThread::msleep(100);
        if (!isProcessAlive(pid)) break;
    }
    if (isProcessAlive(pid))
        ::kill(static_cast<pid_t>(pid), SIGKILL);
    pid = 0;
    ownsBackend = false;
}



void BackendManager::beginShutdown()
{
    shuttingDown.store(true);
    recovering.store(false);
}

void BackendManager::recover()
{
    if (shuttingDown.load()) {
        qDebug() << "[BackendManager] Recovery skipped during shutdown";
        return;
    }

    bool expected = false;
    if (!recovering.compare_exchange_strong(expected, true)) {
        qDebug() << "[BackendManager] Recovery already in progress, skipping";
        return;
    }

    static constexpr int kDelayMs = 2000;
    qDebug() << "[BackendManager] WebSocket lost, recovery in" << kDelayMs << "ms";

    QTimer::singleShot(kDelayMs, this, [this]() {
        if (shuttingDown.load()) {
            qDebug() << "[BackendManager] Recovery cancelled during shutdown";
            recovering.store(false);
            return;
        }

        if (isRunning()) {
            qDebug() << "[BackendManager] Process still alive — transient disconnect";
            recovering.store(false);
            return;
        }

        qDebug() << "[BackendManager] Process gone — starting recovery";
        start();
        recovering.store(false);
    });
}

bool BackendManager::start()
{
    if (isRunning()) {
        qDebug() << "[BackendManager] Backend already running, attach only";
        ownsBackend = false;
        return true;
    }

    const QString path = binaryPath();
    if (path.isEmpty()) {
        qDebug() << "[BackendManager] Startup failed: binary not found";
        return false;
    }

    qDebug() << "[BackendManager] Starting backend:" << path;

    QProcess process;
    process.setProgram(path);
    process.setWorkingDirectory(QFileInfo(path).absolutePath());
    process.setProcessEnvironment(backendProcessEnvironment());

    qint64 newPid = 0;
    if (!process.startDetached(&newPid) || newPid <= 0) {
        qDebug() << "[BackendManager] Startup failed: startDetached returned false";
        return false;
    }

    pid = newPid;
    ownsBackend = true;
    qDebug() << "[BackendManager] Started backend successfully (pid" << pid << ")";
    return true;
}

bool BackendManager::restart()
{
    qDebug() << "[BackendManager] restart";

    if (ownsBackend && pid > 0 && isProcessAlive(pid)) {
        qDebug() << "[BackendManager] restart: stopping owned backend (pid" << pid << ")";
        stopProcess();
        QThread::msleep(500);
    } else {
        qDebug() << "[BackendManager] restart: backend is shared, not stopping it";
    }

    return start();
}

bool BackendManager::stop()
{
    if (!ownsBackend || pid <= 0) {
        qDebug() << "[BackendManager] stop: backend is shared or not running, skipping";
        return false;
    }
    qDebug() << "[BackendManager] stop: stopping owned backend (pid" << pid << ")";
    stopProcess();
    return true;
}

QString BackendManager::version()
{
    const QString path = binaryPath();
    if (path.isEmpty()) return {};

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.setWorkingDirectory(QFileInfo(path).absolutePath());
    proc.setProcessEnvironment(backendProcessEnvironment());
    proc.start(path, {QStringLiteral("-version")});
    if (!proc.waitForStarted(3000) || !proc.waitForFinished(5000)) {
        proc.kill();
        return {};
    }

    const QString out = QString::fromUtf8(proc.readAll()).trimmed();
    static const QRegularExpression re(QStringLiteral(R"(magicpodscore\s+(\d+(?:\.\d+)*))"));
    const auto match = re.match(out);
    return match.hasMatch() ? match.captured(1) : QString{};
}
