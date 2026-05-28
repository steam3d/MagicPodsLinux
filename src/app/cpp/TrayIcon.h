// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <optional>

#include <QSystemTrayIcon>

class TrayIcon final : public QSystemTrayIcon {
    Q_OBJECT
    Q_PROPERTY(int themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)

public:
    enum class IconType {
        Default,
        Warning
    };

    enum class ThemeMode {
        Auto  = 0,
        Light = 1,
        Dark  = 2
    };

    explicit TrayIcon(QObject *parent = nullptr);

    void setIconType(IconType type);
    IconType iconType() const;
    void setTextIcon(int value);
    void clearTextIcon();

    int themeMode() const;
    void setThemeMode(int mode);

signals:
    void leftClicked();
    void rightClicked();
    void middleClicked();
    void themeModeChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool isDarkTheme() const;
    QString iconFileName(IconType type) const;
    QIcon themedIcon(IconType type) const;
    QIcon themedTextIcon() const;
    void updateIcon();

    IconType m_iconType = IconType::Default;
    ThemeMode m_themeMode = ThemeMode::Auto;
    std::optional<int> m_textIconValue;
};
