// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import "pages"
import "components" as Components
import magicpods as MP

QQC2.ApplicationWindow {
    id: root
    minimumWidth: gameScopeMode ? 0 : 488
    width: gameScopeMode ? Screen.width : 700
    height: gameScopeMode ? Screen.height : 460
    visibility: gameScopeMode ? Window.FullScreen : Window.Windowed
    visible: true

    property alias currentIndex: swipeView.currentIndex
    onClosing: (event) => {
        event.accepted = false;
        root.visible = false;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        QQC2.ProgressBar {
            visible: !(cppBackend?.connected ?? false) && !(cppBackend?.unsupportedApi ?? false)
            indeterminate: true
            Layout.fillWidth: true

            Layout.leftMargin: -1
            Layout.rightMargin: -1
            Layout.topMargin: 1
            Layout.bottomMargin: 0
        }

        Components.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: MP.Units.mediumSpacing
            visible: cppBackend?.unsupportedApi ?? false
            type: 2
            text: qsTrId("Error.service_api_wrong")
        }

        QQC2.SwipeView {
            id: swipeView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            BatteryPage {}
            HeadphonesPage {}
            SettingsPage {}
            AboutPage {}
        }
    }

    footer: QQC2.TabBar {
        id: footer
        currentIndex: swipeView.currentIndex
        bottomPadding: MP.Units.mediumSpacing

        background: Rectangle {
            color: footer.palette.window
            Components.Separator {
                width: parent.width
            }
        }

        onCurrentIndexChanged: {
            if (swipeView.currentIndex !== currentIndex) {
                swipeView.currentIndex = currentIndex;
            }
        }

        QQC2.TabButton {
            id: tabBattery
            text: qsTrId("menu.battery")
            contentItem: Item {
                Row {
                    anchors.centerIn: parent
                    spacing: 8
                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/menu-battery.png"
                        width: 24; height: 24
                        fillMode: Image.PreserveAspectFit
                        mipmap: true
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: tabBattery.text
                        font: tabBattery.font
                        color: tabBattery.palette.buttonText
                    }
                }
            }
        }

        QQC2.TabButton {
            id: tabHeadphones
            text: qsTrId("menu.headphones")
            contentItem: Item {
                Row {
                    anchors.centerIn: parent
                    spacing: 8
                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/menu-headphones.png"
                        width: 24; height: 24
                        fillMode: Image.PreserveAspectFit
                        mipmap: true
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: tabHeadphones.text
                        font: tabHeadphones.font
                        color: tabHeadphones.palette.buttonText
                    }
                }
            }
        }

        QQC2.TabButton {
            id: tabSettings
            text: qsTrId("menu.settings")
            contentItem: Item {
                Row {
                    anchors.centerIn: parent
                    spacing: 8
                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/menu-settigns.png"
                        width: 24; height: 24
                        fillMode: Image.PreserveAspectFit
                        mipmap: true
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: tabSettings.text
                        font: tabSettings.font
                        color: tabSettings.palette.buttonText
                    }
                }
            }
        }

        QQC2.TabButton {
            id: tabAbout
            text: qsTrId("menu.about")
            contentItem: Item {
                Row {
                    anchors.centerIn: parent
                    spacing: 8
                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/menu-help.png"
                        width: 24; height: 24
                        fillMode: Image.PreserveAspectFit
                        mipmap: true
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: tabAbout.text
                        font: tabAbout.font
                        color: tabAbout.palette.buttonText
                    }
                }
            }
        }
    }
}
