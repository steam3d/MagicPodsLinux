// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import magicpods as MP
import "../components" as Components

QQC2.Page {
    id: rootPage
    padding: 0
    background: Rectangle {
        color: "transparent"
    }

    title: qsTrId("menu.headphones")

    readonly property int mWidth: MP.Units.gridUnit * 12
    property var btAdapterData: ({})
    property var headphonesData: []
    readonly property var sortedHeadphones: hasHeadphones ? headphonesData.slice().sort(function (a, b) {
        return (a.name || "").localeCompare(b.name || "");
    }) : []
    readonly property bool hasHeadphones: Object.keys(headphonesData).length > 0
    readonly property bool hasBtAdapter: Object.keys(btAdapterData).length > 0

    function requestDevicesData() {
        if (cppBackend && cppBackend.connected) {
            cppBackend.getDefaultBluetoothAdapter();
            cppBackend.getDevices();
        }
    }

    Connections {
        target: cppBackend
        enabled: !!cppBackend
        function onDataReceived(json) {
            if (!json || Object.keys(json).length === 0) {
                rootPage.headphonesData = [];
            } else if (json.headphones) {
                rootPage.headphonesData = json.headphones;
            }

            if (!json || Object.keys(json).length === 0) {
                rootPage.btAdapterData = ({});
            } else if (json.defaultbluetooth) {
                rootPage.btAdapterData = json.defaultbluetooth;
            }
        }
        function onConnectedChanged() {
            requestDevicesData();
        }
    }
    Component.onCompleted: {
        requestDevicesData();
    }

    Components.HelpMessage {
        visible: !hasBtAdapter
        iconSource: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/bluetooth-not-found.png"
        titleText: qsTrId("headphones.help.bluetooth_not_found.header")
        bodyText: qsTrId("headphones.help.bluetooth_not_found.description")
        width: mWidth * 1.5
    }

    Components.HelpMessage {
        visible: !hasHeadphones && hasBtAdapter
        iconSource: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/pair-headphones.png"
        titleText: qsTrId("headphones.help.bluetooth_no_paired_headphones.header")
        bodyText: qsTrId("headphones.help.no_paired_headphones.description")
        width: mWidth * 1.5
    }

    QQC2.ScrollView {
        id: headphonesScrollView
        anchors.fill: parent
        visible: hasHeadphones && hasBtAdapter
        contentWidth: availableWidth

        ColumnLayout {
            width: headphonesScrollView.availableWidth
            y: Math.max(0, (headphonesScrollView.availableHeight - implicitHeight) / 2)
            spacing: MP.Units.mediumSpacing

            MP.Label {
                Layout.fillWidth: true
                Layout.topMargin: MP.Units.largeSpacing
                Layout.bottomMargin: MP.Units.smallSpacing
                font.bold: true
                text: qsTrId("headphones.bluetooth")
                horizontalAlignment: Text.AlignHCenter
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                visible: rootPage.hasBtAdapter
                label: qsTrId("headphones.item.bluetooth")

                QQC2.Switch {
                    id: bt
                    checked: rootPage.btAdapterData?.enabled ?? false
                    onToggled: {
                        rootPage.btAdapterData.enabled = checked;
                        if (checked)
                            cppBackend.enableDefaultBluetoothAdapter();
                        else
                            cppBackend.disableDefaultBluetoothAdapter();
                    }
                }
            }

            MP.Label {
                Layout.fillWidth: true
                Layout.topMargin: MP.Units.largeSpacing
                Layout.bottomMargin: MP.Units.smallSpacing
                font.bold: true
                text: qsTrId("headphones.headphones")
                horizontalAlignment: Text.AlignHCenter
            }

            Repeater {
                model: rootPage.sortedHeadphones
                delegate: MP.FormRow {
                    Layout.fillWidth: true
                    Layout.maximumWidth: mWidth * 1.5
                    Layout.alignment: Qt.AlignHCenter
                    enabled: bt.checked
                    label: modelData.name + ":"

                    QQC2.Switch {
                        checked: modelData.connected
                        onToggled: {
                            if (modelData.address) {
                                modelData.connected = checked;
                                if (checked)
                                    cppBackend.connectDevice(modelData.address);
                                else
                                    cppBackend.disconnectDevice(modelData.address);
                            }
                        }
                    }
                }
            }

            Item {
                Layout.preferredHeight: 0
            }
        }
    }
}
