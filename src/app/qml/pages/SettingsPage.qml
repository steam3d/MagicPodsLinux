// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import magicpods as MP

QQC2.Page {
    id: rootPage
    padding: 0
    background: Rectangle {
        color: "transparent"
    }

    property bool settingAnimation: true

    readonly property int mWidth: MP.Units.gridUnit * 12
    title: qsTrId("menu.settings")

    function themeTrayIndexToString(index) {
        if (index === 1) return "light";
        if (index === 2) return "dark";
        return "auto";
    }

    function applicationThemeIndex(theme) {
        if (theme === "light") return 1;
        if (theme === "dark") return 2;
        return 0;
    }

    function applicationThemeIndexToString(index) {
        if (index === 1) return "light";
        if (index === 2) return "dark";
        return "system";
    }

    function languageIndex(language) {
        const languages = languageManager?.availableLanguages ?? [];
        for (let index = 0; index < languages.length; ++index) {
            if (languages[index].code === language)
                return index;
        }
        return 0;
    }

    function gameModeScaleIndex(scale) {
        const scales = ["off", "1.0", "1.25", "1.5", "1.75", "2.0"];
        const index = scales.indexOf(scale);
        return index >= 0 ? index : 3;
    }

    function requestSettings() {
        if (cppBackend && cppBackend.connected) {
            cppBackend.getSetting("magicpods", "animation");
        }
    }

    Connections {
        target: cppBackend
        enabled: !!cppBackend
        function onDataReceived(json) {
            if (!json || Object.keys(json).length === 0) {} else if (json.settings) {
                if (json.settings?.magicpods?.animation != null)
                    rootPage.settingAnimation = json.settings.magicpods.animation;
            }
        }
        function onConnectedChanged() {
            requestSettings();
        }
    }
    Component.onCompleted: {
        requestSettings();
    }

    QQC2.ScrollView {
        id: settingsScrollView
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: settingsScrollView.availableWidth
            y: Math.max(0, (settingsScrollView.availableHeight - implicitHeight) / 2)
            spacing: MP.Units.mediumSpacing

            MP.Label {
                Layout.fillWidth: true
                Layout.topMargin: MP.Units.largeSpacing
                Layout.bottomMargin: MP.Units.smallSpacing
                font.bold: true
                text: qsTrId("settings.popup")
                horizontalAlignment: Text.AlignHCenter
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("settings.headphones_animation")
                tooltip: qsTrId("settings.headphones_animation.description")

                QQC2.Switch {
                    checked: settingAnimation
                    enabled: cppBackend?.connected ?? false
                    onToggled: {
                        if (settingAnimation !== checked) {
                            settingAnimation = checked;
                            if (cppBackend)
                                cppBackend.setSetting("magicpods", "animation", checked);
                        }
                    }
                }
            }

            MP.Label {
                Layout.fillWidth: true
                Layout.topMargin: MP.Units.largeSpacing
                Layout.bottomMargin: MP.Units.smallSpacing
                font.bold: true
                text: qsTrId("settings.other")
                horizontalAlignment: Text.AlignHCenter
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("settings.language")

                QQC2.ComboBox {
                    width: rootPage.mWidth
                    model: languageManager?.availableLanguages ?? []
                    textRole: "name"
                    valueRole: "code"
                    currentIndex: rootPage.languageIndex(languageManager?.language ?? "system")
                    onActivated: {
                        if (languageManager)
                            languageManager.language = currentValue;
                    }
                }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("settings.game_mode_scale")
                tooltip: qsTrId("settings.game_mode_scale.description")

                QQC2.ComboBox {
                    width: rootPage.mWidth
                    model: [
                        { text: qsTrId("settings.game_mode_scale.off"), value: "off" },
                        { text: "100%", value: "1.0" },
                        { text: "125%", value: "1.25" },
                        { text: "150%", value: "1.5" },
                        { text: "175%", value: "1.75" },
                        { text: "200%", value: "2.0" }
                    ]
                    textRole: "text"
                    valueRole: "value"
                    currentIndex: rootPage.gameModeScaleIndex(
                                      uiSettings?.value("GameMode/scale", "1.5") ?? "1.5")
                    onActivated: {
                        if (uiSettings)
                            uiSettings.setValue("GameMode/scale", currentValue);
                    }
                }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("settings.add_shortcut_to_menu")
                tooltip: qsTrId("settings.add_shortcut_to_menu.description")

                QQC2.Switch {
                    id: menuShortcutSwitch
                    property bool skipToggle: false
                    checked: false
                    enabled: false

                    function refreshState() {
                        skipToggle = true;
                        enabled = !!desktopManager;
                        checked = !!desktopManager && desktopManager.isInstalled();
                        skipToggle = false;
                    }

                    onToggled: {
                        if (skipToggle)
                            return;

                        if (checked) {
                            if (!desktopManager?.install())
                                refreshState();
                        } else {
                            if (!desktopManager?.uninstall())
                                refreshState();
                        }
                    }

                    Component.onCompleted: refreshState()
                }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("settings.application_theme")

                QQC2.ComboBox {
                    width: rootPage.mWidth
                    // TODO: Enable this in GameScope by applying an explicit light/dark palette
                    // to ApplicationWindow and clearing it for System. GameScope's Wayland
                    // platform ignores QStyleHints color-scheme overrides and reports Unknown.
                    enabled: !gameScopeMode
                    model: [
                        qsTrId("settings.theme.system"),
                        qsTrId("settings.theme.light"),
                        qsTrId("settings.theme.dark")
                    ]
                    currentIndex: rootPage.applicationThemeIndex(
                                      uiSettings?.value("Theme/application", "system") ?? "system")
                    onActivated: {
                        if (uiSettings)
                            uiSettings.setValue("Theme/application",
                                                rootPage.applicationThemeIndexToString(currentIndex));
                    }
                }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("settings.tray_icon_theme")

                QQC2.ComboBox {
                    width: rootPage.mWidth
                    enabled: !!cppTrayIcon
                    model: [
                        qsTrId("settings.theme.system"),
                        qsTrId("settings.theme.light"),
                        qsTrId("settings.theme.dark")
                    ]
                    currentIndex: cppTrayIcon ? cppTrayIcon.themeMode : 0
                    onActivated: {
                        if (cppTrayIcon)
                            cppTrayIcon.themeMode = currentIndex;
                        if (uiSettings)
                            uiSettings.setValue("Theme/tray_icon", rootPage.themeTrayIndexToString(currentIndex));
                    }
                }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("settings.exit")
                tooltip: qsTrId("settings.exit.description")

                QQC2.Button {
                    text: qsTrId("settings.exit_button")
                    onClicked: Qt.quit()
                }
            }

            Item {
                Layout.preferredHeight: 0
            }
        }
    }
}
