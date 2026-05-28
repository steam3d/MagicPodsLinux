// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2

QQC2.Control {
    id: batteryRow
    property int battery: 0
    property bool isCharging: false
    property int status: 2
    property string name: ""
    spacing: 8
    visible: status !== 0 && status !== 1
    opacity: status === 3 ? 0.5 : 1
    padding: 0

    property color fillColor: "#59BF40"
    property color textColor: palette.windowText
    readonly property string _textColorHex: Qt.rgba(textColor.r, textColor.g, textColor.b, 1).toString()
    property real _level: Math.max(0, Math.min(100, battery)) / 100
    property real _fillWidth: 20 * _level
    property string _svg: (`<svg width="16" height="28" viewBox="0 0 16 28" fill="none" xmlns="http://www.w3.org/2000/svg"><g clip-path="url(#clip0_2136_1101)"><path fill-rule="evenodd" clip-rule="evenodd" d="M11 0H5V2H0V28H16V2H11V0ZM1.5 3.5H14.5V26.5H1.5V3.5Z" fill="${_textColorHex}"/><rect x="3" y="${5 + (20 - _fillWidth)}" width="10" height="${_fillWidth}" fill="${fillColor}"/><path d="M4.5 16.25L8 10V13.75H11.5L8 20V16.25H4.5Z" fill="${_textColorHex}" opacity="${isCharging ? 1 : 0}"/></g><defs><clipPath id="clip0_2136_1101"><rect width="16" height="28" fill="white"/></clipPath></defs></svg>`)

    contentItem: Row {
        spacing: batteryRow.spacing

        Image {
            id: batteryIcon
            width: 16
            height: 28
            sourceSize.width: 16 * Screen.devicePixelRatio
            sourceSize.height: 28 * Screen.devicePixelRatio
            source: "data:image/svg+xml;utf8," + encodeURIComponent(batteryRow._svg)
            smooth: false
        }

        ColumnLayout {
            spacing: 0
            Text {
                text: batteryRow.name
                color: batteryRow.textColor
                font.pixelSize: 12
            }
            Text {
                Layout.topMargin: -4
                text: batteryRow.battery + "%"
                color: batteryRow.textColor
                font.pixelSize: 14
                font.bold: true
            }
        }
    }
}
