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

    readonly property int mWidth: MP.Units.gridUnit * 12
    title: qsTrId("menu.shortcuts")

    QQC2.Dialog {
        id: configurationHelpDialog
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(420, parent ? parent.width - 32 : 420)
        modal: true
        focus: true
        title: qsTrId("shortcuts.configure_help.header")
        standardButtons: QQC2.Dialog.Ok

        contentItem: QQC2.Label {
            text: qsTrId("shortcuts.configure_help.description").arg(
                      qsTrId("menu.shortcuts"))
            wrapMode: Text.WordWrap
        }
    }

    Connections {
        target: globalShortcutManager

        function onConfigurationHelpRequested() {
            configurationHelpDialog.open();
        }
    }

    MP.HelpMessage {
        visible: !gameScopeMode && !globalShortcutManager.available
        iconSource: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/menu-bandage.png"
        titleText: qsTrId("shortcuts.portal_unavailable.header")
        bodyText: qsTrId("shortcuts.portal_unavailable.description")
        width: rootPage.mWidth * 1.5
    }

    MP.HelpMessage {
        visible: globalShortcutManager.available
                 && !gameScopeMode
                 && !globalShortcutManager.desktopEntryAvailable
        iconSource: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/add-shortcut.png"
        titleText: qsTrId("shortcuts.desktop_entry_missing.header")
        bodyText: qsTrId("shortcuts.desktop_entry_missing.description")
        actionText: qsTrId("shortcuts.desktop_entry_missing.action")
        width: rootPage.mWidth * 1.5

        onActionTriggered: {
            if (desktopManager?.install())
                Qt.quit();
        }
    }

    MP.HelpMessage {
        visible: gameScopeMode
        iconSource: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/menu-shortcuts.png"
        titleText: qsTrId("shortcuts.game_scope.header")
        bodyText: qsTrId("shortcuts.game_scope.description")
        width: rootPage.mWidth * 1.5
    }

    QQC2.ScrollView {
        id: shortcutsScrollView
        anchors.fill: parent
        visible: globalShortcutManager.available
                 && globalShortcutManager.desktopEntryAvailable
                 && !gameScopeMode
        contentWidth: availableWidth

        ColumnLayout {
            width: shortcutsScrollView.availableWidth
            y: Math.max(0, (shortcutsScrollView.availableHeight - implicitHeight) / 2)
            spacing: MP.Units.mediumSpacing

            MP.Label {
                Layout.fillWidth: true
                Layout.topMargin: MP.Units.largeSpacing
                Layout.bottomMargin: MP.Units.smallSpacing
                font.bold: true
                text: qsTrId("shortcuts.system_settings")
                horizontalAlignment: Text.AlignHCenter
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: rootPage.mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("shortcuts.setting")
                tooltip: qsTrId("shortcuts.configure.description")

                QQC2.Button {
                    width: rootPage.mWidth
                    text: qsTrId("shortcuts.configure")
                    enabled: globalShortcutManager.canConfigure
                             && !globalShortcutManager.busy
                    onClicked: globalShortcutManager.configureShortcuts()
                }
            }

            MP.Label {
                Layout.fillWidth: true
                Layout.topMargin: MP.Units.largeSpacing
                Layout.bottomMargin: MP.Units.smallSpacing
                font.bold: true
                text: qsTrId("shortcuts.actions")
                horizontalAlignment: Text.AlignHCenter
            }

            Repeater {
                model: 10

                delegate: MP.FormRow {
                    required property int index
                    readonly property int slotNumber: index + 1
                    readonly property string slotId: "slot-"
                                                     + (slotNumber < 10 ? "0" : "")
                                                     + slotNumber

                    Layout.fillWidth: true
                    Layout.maximumWidth: rootPage.mWidth * 1.5
                    Layout.alignment: Qt.AlignHCenter
                    label: qsTrId("shortcuts.slot").arg(slotNumber)

                    QQC2.ComboBox {
                        id: actionComboBox
                        width: rootPage.mWidth
                        model: shortcutActionManager.availableActions
                        textRole: "text"
                        currentIndex: shortcutActionManager.assignmentIndex(slotId)
                        hoverEnabled: true
                        popup.width: rootPage.mWidth * 1.75

                        function refreshSelection() {
                            currentIndex = shortcutActionManager.assignmentIndex(slotId);
                        }

                        onActivated: shortcutActionManager.setAssignmentFromIndex(
                                         slotId, currentIndex)

                        QQC2.ToolTip.visible: hovered
                                                  && displayText.length > 0
                        QQC2.ToolTip.delay: 250
                        QQC2.ToolTip.timeout: 10000
                        QQC2.ToolTip.text: displayText

                        Connections {
                            target: shortcutActionManager

                            function onAvailableActionsChanged() {
                                // Let ComboBox update its model before restoring the selection.
                                Qt.callLater(actionComboBox.refreshSelection);
                            }
                        }
                    }
                }
            }

            Item {
                Layout.preferredHeight: MP.Units.smallSpacing
            }
        }
    }
}
