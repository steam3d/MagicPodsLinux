// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick
import QtQuick.Controls 2.15

Control {
    id: batteryRow

    property int battery: 0
    property bool isCharging: false
    property int status: 2

    visible: status !== 0 && status !== 1
    opacity: status === 3 ? 0.5 : 1
    padding: 0

    property color fillColor: "#59BF40"
    property color textColor: palette.windowText
    property color strokeColor: palette.window
    property color formColor: "#C2C2C2"
    readonly property string _textColorHex: Qt.rgba(textColor.r, textColor.g, textColor.b, 1).toString()
    readonly property string _strokeColorHex: Qt.rgba(strokeColor.r, strokeColor.g, strokeColor.b, 1).toString()
    property real _level: Math.max(0, Math.min(100, battery)) / 100
    property real _fillWidth: 18 * _level
    property string _svg: (`<svg width="24" height="12" viewBox="0 0 24 12" fill="none" xmlns="http://www.w3.org/2000/svg"><g clip-path="url(#clip0_2132_1100)"><path d="M19 0C20.6569 0 22 1.34315 22 3V4C23.1045 4 24 4.89543 24 6C24 7.10457 23.1045 8 22 8V9C22 10.6569 20.6569 12 19 12H3C1.34315 12 1.61064e-08 10.6569 0 9V3C0 1.34315 1.34315 0 3 0H19ZM3 1C1.89543 1 1 1.89543 1 3V9L1.01074 9.2041C1.113 10.2128 1.96435 11 3 11H19C20.1046 11 21 10.1046 21 9V3C21 1.89543 20.1046 1 19 1H3Z" fill="${formColor}"/><rect x="2" y="2" width="${_fillWidth.toFixed(2)}" height="8" rx="1" fill="${fillColor}"/><path d="M13.3737 2.70487C13.7072 1.36815 12.0253 0.52754 11.1501 1.51541L11.0681 1.616L8.25751 5.40897C7.6223 6.26688 8.23482 7.48221 9.30243 7.48221H9.43817L9.01435 9.27518C8.68584 10.6644 10.5115 11.4914 11.3386 10.3279L14.1852 6.32303C14.7968 5.46237 14.1815 4.27047 13.1257 4.2703H12.9831L13.3737 2.70487Z" fill="${_strokeColorHex}" opacity="${isCharging ? 1 : 0}"/><path d="M10.6676 6.62749L9.98721 9.50492C9.9114 9.82552 10.3328 10.0163 10.5237 9.74775L13.3701 5.74296C13.5113 5.54433 13.3692 5.26917 13.1256 5.26917H11.8726C11.8312 5.26917 11.7927 5.25276 11.7661 5.22293C11.7402 5.19459 11.7305 5.15766 11.7394 5.12191L12.4035 2.46206C12.483 2.14361 12.0668 1.94707 11.8714 2.21079L9.06145 6.00313C8.91475 6.20111 9.05608 6.48173 9.30249 6.48173H10.5343C10.5748 6.48173 10.6147 6.4988 10.6398 6.5257C10.6659 6.55461 10.676 6.59179 10.6676 6.62749Z" fill="${_textColorHex}" opacity="${isCharging ? 1 : 0}"/></g><defs><clipPath id="clip0_2132_1100"><rect width="24" height="12" fill="${_strokeColorHex}"/></clipPath></defs></svg>`)

    contentItem: Column {
        id: batteryColumn
        width: Math.max(batteryIcon.width, batteryLabel.implicitWidth)

        Image {
            id: batteryIcon
            anchors.horizontalCenter: parent.horizontalCenter
            width: 24
            height: 12
            sourceSize.width: 24 * Screen.devicePixelRatio
            sourceSize.height: 12 * Screen.devicePixelRatio
            source: "data:image/svg+xml;utf8," + encodeURIComponent(batteryRow._svg)
            smooth: false
        }

        Text {
            id: batteryLabel
            text: batteryRow.battery + "%"
            width: parent.width
            horizontalAlignment: Qt.AlignHCenter
            color: batteryRow.palette.windowText
            font.pixelSize: 12
        }
    }
}
