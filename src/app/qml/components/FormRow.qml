// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

RowLayout {
    id: root

    property string label: ""
    property string tooltip: ""
    default property alias content: contentHolder.data

    spacing: 16

    Item {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        implicitHeight: labelText.implicitHeight

        Text {
            id: labelText
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: root.label
            color: palette.windowText
            font.pixelSize: 14
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        implicitHeight: rightRow.implicitHeight

        RowLayout {
            id: rightRow
            anchors.left: parent.left
            anchors.top: parent.top
            spacing: 4

            Item {
                id: contentHolder
                implicitWidth: childrenRect.width
                implicitHeight: childrenRect.height
            }

            ToolButton {
                id: tooltipBtn
                visible: root.tooltip !== ""
                Layout.alignment: Qt.AlignVCenter
                icon.source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/icon-help.svg"
                icon.color: palette.buttonText
                hoverEnabled: true

                ToolTip.visible: hovered
                ToolTip.delay: 250
                ToolTip.timeout: 10000
                ToolTip.text: root.tooltip
            }
        }
    }
}
