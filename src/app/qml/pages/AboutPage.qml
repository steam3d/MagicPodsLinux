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

    title: qsTrId("menu.about")
    readonly property int mWidth: MP.Units.gridUnit * 12

    QQC2.ScrollView {
        id: aboutScrollView
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: aboutScrollView.availableWidth
            y: Math.max(0, (aboutScrollView.availableHeight - implicitHeight) / 2)

            RowLayout {
                Layout.topMargin: MP.Units.largeSpacing
                Layout.alignment: Qt.AlignHCenter
                spacing: MP.Units.largeSpacing
                Layout.fillWidth: false

                Image {
                    Layout.preferredHeight: 64
                    Layout.preferredWidth: 64
                    source: "qrc:/qt/qml/magicpods/src/app/qml/assets/images/logo-512.png"
                    smooth: true
                    mipmap: true
                }

                ColumnLayout {
                    MP.Heading {
                        level: 1
                        text: "MagicPods"
                        wrapMode: Text.WordWrap
                        font.bold: true
                        font.pointSize: Qt.application.font.pointSize * 2
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        maximumLineCount: 1
                    }

                    MP.Label {
                        Layout.topMargin: -4
                        text: qsTrId("about.button.home_page")
                        color: palette.link
                        font.underline: true
                        MouseArea {
                            id: mouseHomePage
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: Qt.openUrlExternally("https://magicpods.app")
                        }
                        QQC2.ToolTip {
                            visible: mouseHomePage.containsMouse
                            text: "https://magicpods.app"
                            delay: 500
                        }
                    }
                }
            }

            MP.Separator {
                Layout.maximumWidth: mWidth * 1.5
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: MP.Units.mediumSpacing
            }

            MP.Label {
                Layout.fillWidth: true
                Layout.topMargin: MP.Units.largeSpacing
                Layout.bottomMargin: MP.Units.smallSpacing
                font.bold: true
                text: qsTrId("about.packages")
                horizontalAlignment: Text.AlignHCenter
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: "MagicPods:"
                MP.Label { text: Qt.application.version }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: "MagicPodsCore:"
                MP.Label {
                    text: backendManager ? backendManager.version() || qsTrId("about.not_installed") : ""
                }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: ""
                MP.Label {
                    text: (cppBackend && cppBackend.backendInfoText !== "")
                          ? cppBackend.backendInfoText
                          : qsTrId("about.not_connected")
                }
            }

            MP.FormRow {
                id: deckyRow                
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: "MagicPodsDecky:"

                MP.Label {
                    text: deckyManager ? deckyManager.version() || qsTrId("about.not_installed") : ""
                }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: ""
                MP.Label {                    
                }
            }

            MP.Label {
                Layout.fillWidth: true
                Layout.topMargin: MP.Units.largeSpacing
                Layout.bottomMargin: MP.Units.smallSpacing
                font.bold: true
                text: qsTrId("about.support")
                horizontalAlignment: Text.AlignHCenter
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("about.item.issue")
                MP.Label {
                    text: "GitHub"
                    color: palette.link
                    font.underline: true
                    MouseArea {
                        id: mouseGitHub
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: Qt.openUrlExternally("https://github.com/steam3d/MagicPodsLinux/issues")
                    }
                    QQC2.ToolTip {
                        visible: mouseGitHub.containsMouse
                        text: "https://github.com/steam3d/MagicPodsLinux/issues"
                        delay: 500
                    }
                }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: qsTrId("about.item.community")
                MP.Label {
                    text: "Discord"
                    color: palette.link
                    font.underline: true
                    MouseArea {
                        id: mouseDiscord
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: Qt.openUrlExternally("https://discord.com/invite/UyY4PY768V")
                    }
                    QQC2.ToolTip {
                        visible: mouseDiscord.containsMouse
                        text: "https://discord.com/invite/UyY4PY768V"
                        delay: 500
                    }
                }
            }

            MP.FormRow {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                label: ""
                MP.Label {
                    text: "Telegram"
                    color: palette.link
                    font.underline: true
                    MouseArea {
                        id: mouseTelegram
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: Qt.openUrlExternally("https://t.me/magicpods")
                    }
                    QQC2.ToolTip {
                        visible: mouseTelegram.containsMouse
                        text: "https://t.me/magicpods"
                        delay: 500
                    }
                }
            }

            Item {
                Layout.preferredHeight: 0
            }
        }
    }
}
