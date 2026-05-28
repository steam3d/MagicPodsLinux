// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

QQC2.ApplicationWindow {
    id: root
    width: 700
    height: 500
    minimumWidth: 400
    visible: true
    title: "Color Sample"

    onClosing: (event) => {
        event.accepted = false
        root.visible = false
    }

    QQC2.Control {
        id: paletteActive
        visible: false
        enabled: true
    }
    QQC2.Control {
        id: paletteDisabled
        visible: false
        enabled: false
    }

    Flickable {
        anchors.fill: parent
        contentHeight: paletteCol.implicitHeight
        contentWidth: width
        clip: true

        Column {
            id: paletteCol
            width: parent.width
            topPadding: 16
            bottomPadding: 16
            leftPadding: 16
            rightPadding: 16
            spacing: 16

            component ColorSwatch: Column {
                property color swatchColor: "transparent"
                property string swatchName: ""
                spacing: 4
                width: 100

                Rectangle {
                    width: 92
                    height: 52
                    color: swatchColor
                    border.color: Qt.rgba(0, 0, 0, 0.2)
                    border.width: 1
                    radius: 4

                    QQC2.Label {
                        anchors.centerIn: parent
                        text: swatchColor.toString().toUpperCase()
                        font.pixelSize: 9
                        color: {
                            var c = swatchColor
                            return (0.299 * c.r + 0.587 * c.g + 0.114 * c.b) > 0.5 ? "#000000" : "#ffffff"
                        }
                    }
                }

                QQC2.Label {
                    width: 92
                    text: swatchName
                    font.pixelSize: 10
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            QQC2.Label {
                text: "Active (enabled)"
                font.pixelSize: 15
                font.bold: true
            }

            Flow {
                width: parent.width - 32
                spacing: 8

                Repeater {
                    model: [
                        { name: "window",           color: paletteActive.palette.window },
                        { name: "windowText",       color: paletteActive.palette.windowText },
                        { name: "base",             color: paletteActive.palette.base },
                        { name: "alternateBase",    color: paletteActive.palette.alternateBase },
                        { name: "text",             color: paletteActive.palette.text },
                        { name: "placeholderText",  color: paletteActive.palette.placeholderText },
                        { name: "brightText",       color: paletteActive.palette.brightText },
                        { name: "button",           color: paletteActive.palette.button },
                        { name: "buttonText",       color: paletteActive.palette.buttonText },
                        { name: "light",            color: paletteActive.palette.light },
                        { name: "midlight",         color: paletteActive.palette.midlight },
                        { name: "mid",              color: paletteActive.palette.mid },
                        { name: "dark",             color: paletteActive.palette.dark },
                        { name: "shadow",           color: paletteActive.palette.shadow },
                        { name: "highlight",        color: paletteActive.palette.highlight },
                        { name: "highlightedText",  color: paletteActive.palette.highlightedText },
                        { name: "accent",           color: paletteActive.palette.accent },
                        { name: "link",             color: paletteActive.palette.link },
                        { name: "linkVisited",      color: paletteActive.palette.linkVisited },
                        { name: "toolTipBase",      color: paletteActive.palette.toolTipBase },
                        { name: "toolTipText",      color: paletteActive.palette.toolTipText }
                    ]
                    delegate: ColorSwatch {
                        swatchColor: modelData.color
                        swatchName: modelData.name
                    }
                }
            }

            QQC2.Label {
                text: "Disabled (enabled: false)"
                font.pixelSize: 15
                font.bold: true
            }

            Flow {
                width: parent.width - 32
                spacing: 8

                Repeater {
                    model: [
                        { name: "window",           color: paletteDisabled.palette.window },
                        { name: "windowText",       color: paletteDisabled.palette.windowText },
                        { name: "base",             color: paletteDisabled.palette.base },
                        { name: "alternateBase",    color: paletteDisabled.palette.alternateBase },
                        { name: "text",             color: paletteDisabled.palette.text },
                        { name: "placeholderText",  color: paletteDisabled.palette.placeholderText },
                        { name: "brightText",       color: paletteDisabled.palette.brightText },
                        { name: "button",           color: paletteDisabled.palette.button },
                        { name: "buttonText",       color: paletteDisabled.palette.buttonText },
                        { name: "light",            color: paletteDisabled.palette.light },
                        { name: "midlight",         color: paletteDisabled.palette.midlight },
                        { name: "mid",              color: paletteDisabled.palette.mid },
                        { name: "dark",             color: paletteDisabled.palette.dark },
                        { name: "shadow",           color: paletteDisabled.palette.shadow },
                        { name: "highlight",        color: paletteDisabled.palette.highlight },
                        { name: "highlightedText",  color: paletteDisabled.palette.highlightedText },
                        { name: "accent",           color: paletteDisabled.palette.accent },
                        { name: "link",             color: paletteDisabled.palette.link },
                        { name: "linkVisited",      color: paletteDisabled.palette.linkVisited },
                        { name: "toolTipBase",      color: paletteDisabled.palette.toolTipBase },
                        { name: "toolTipText",      color: paletteDisabled.palette.toolTipText }
                    ]
                    delegate: ColorSwatch {
                        swatchColor: modelData.color
                        swatchName: modelData.name
                    }
                }
            }
        }
    }
}
