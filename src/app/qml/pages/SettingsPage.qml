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

    function themeTrayStringToIndex(value) {
        if (value === "light") return 1;
        if (value === "dark") return 2;
        return 0;
    }

    function themeTrayIndexToString(index) {
        if (index === 1) return "light";
        if (index === 2) return "dark";
        return "auto";
    }

    function requestSettings() {
        if (cppBackend && cppBackend.connected) {
            cppBackend.getSetting("magicpods", "animation");
            cppBackend.getSetting("magicpods", "theme_tray");
        }
    }

    Connections {
        target: cppBackend
        enabled: !!cppBackend
        function onDataReceived(json) {
            if (!json || Object.keys(json).length === 0) {} else if (json.settings) {
                if (json.settings?.magicpods?.animation != null)
                    rootPage.settingAnimation = json.settings.magicpods.animation;
                if (json.settings?.magicpods?.theme_tray != null && cppTrayIcon)
                    cppTrayIcon.themeMode = rootPage.themeTrayStringToIndex(json.settings.magicpods.theme_tray);
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
                label: qsTrId("settings.tray_icon_theme")

                QQC2.ComboBox {
                    width: rootPage.mWidth
                    enabled: cppBackend?.connected ?? false
                    model: [
                        qsTrId("settings.tray_icon_theme.auto"),
                        qsTrId("settings.tray_icon_theme.light"),
                        qsTrId("settings.tray_icon_theme.dark")
                    ]
                    currentIndex: cppTrayIcon ? cppTrayIcon.themeMode : 0
                    onActivated: {
                        if (cppTrayIcon)
                            cppTrayIcon.themeMode = currentIndex;
                        if (cppBackend)
                            cppBackend.setSetting("magicpods", "theme_tray", rootPage.themeTrayIndexToString(currentIndex));
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
