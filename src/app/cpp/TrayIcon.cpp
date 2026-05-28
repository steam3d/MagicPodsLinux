// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "TrayIcon.h"

#include <QApplication>
#include <QEvent>
#include <QPalette>
#include <QDebug>

namespace {
QString iconPath(const QString &name) {
    return QStringLiteral(":/qt/qml/magicpods/src/app/qml/assets/icons/") + name;
}
}

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent) {
    connect(this, &QSystemTrayIcon::activated, this, [this](ActivationReason reason) {
        qDebug() << "TrayIcon activated, reason =" << reason;
        if (reason == QSystemTrayIcon::Trigger) {
            emit leftClicked();
        } else if (reason == QSystemTrayIcon::MiddleClick) {
            emit middleClicked();
        } else {



            emit rightClicked();
        }
    });

    if (qApp) {
        qApp->installEventFilter(this);
    }

    updateIcon();
}

void TrayIcon::setIconType(IconType type) {
    if (m_iconType == type && !m_textIconValue.has_value()) {
        return;
    }
    m_iconType = type;
    m_textIconValue.reset();
    updateIcon();
}

TrayIcon::IconType TrayIcon::iconType() const {
    return m_iconType;
}

void TrayIcon::setTextIcon(int value) {
    const int clampedValue = qBound(0, value, 99);
    if (m_textIconValue == clampedValue) {
        return;
    }
    m_textIconValue = clampedValue;
    updateIcon();
}

void TrayIcon::clearTextIcon() {
    if (!m_textIconValue.has_value()) {
        return;
    }
    m_textIconValue.reset();
    updateIcon();
}

bool TrayIcon::eventFilter(QObject *watched, QEvent *event) {
    if (watched == qApp && event && event->type() == QEvent::ApplicationPaletteChange) {
        updateIcon();
    }
    return QSystemTrayIcon::eventFilter(watched, event);
}

int TrayIcon::themeMode() const {
    return static_cast<int>(m_themeMode);
}

void TrayIcon::setThemeMode(int mode) {
    const auto newMode = static_cast<ThemeMode>(mode);
    if (m_themeMode == newMode)
        return;
    m_themeMode = newMode;
    emit themeModeChanged();
    updateIcon();
}

bool TrayIcon::isDarkTheme() const {
    if (m_themeMode == ThemeMode::Light)
        return false;
    if (m_themeMode == ThemeMode::Dark)
        return true;
    const QColor bg = QApplication::palette().color(QPalette::Window);
    const qreal luminance = (0.2126 * bg.redF()) + (0.7152 * bg.greenF()) + (0.0722 * bg.blueF());
    return luminance < 0.5;
}

QString TrayIcon::iconFileName(IconType type) const {
    const bool darkTheme = isDarkTheme();
    switch (type) {
    case IconType::Warning:
        return darkTheme ? QStringLiteral("logo-warning_dark.svg") : QStringLiteral("logo-warning_light.svg");
    case IconType::Default:
    default:
        return darkTheme ? QStringLiteral("logo-dark.svg") : QStringLiteral("logo-light.svg");
    }
}

QIcon TrayIcon::themedIcon(IconType type) const {
    return QIcon(iconPath(iconFileName(type)));
}

QIcon TrayIcon::themedTextIcon() const {
    if (!m_textIconValue.has_value()) {
        return {};
    }
    const QString themeDir = isDarkTheme() ? QStringLiteral("battery-text-dark") : QStringLiteral("battery-text-light");
    return QIcon(iconPath(QStringLiteral("%1/%2.svg").arg(themeDir, QString::number(*m_textIconValue))));
}

void TrayIcon::updateIcon() {
    if (m_textIconValue.has_value()) {
        const QIcon icon = themedTextIcon();
        if (!icon.isNull()) {
            setIcon(icon);
            return;
        }
    }

    const QIcon icon = themedIcon(m_iconType);
    if (!icon.isNull()) {
        setIcon(icon);
    }
}
