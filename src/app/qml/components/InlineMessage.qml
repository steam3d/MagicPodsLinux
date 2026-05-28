// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

QQC2.Frame {
    id: root

    readonly property int typeInformation: 0
    readonly property int typePositive:    1
    readonly property int typeWarning:     2
    readonly property int typeError:       3

    property int    type:    typeInformation
    property string text:    ""
    property list<QtObject> actions

    contentItem: RowLayout {
        spacing: 8

        Image {
            source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/menu-help.png"
            Layout.preferredWidth:  24
            Layout.preferredHeight: 24
            fillMode:          Image.PreserveAspectFit
            smooth:            true
            mipmap:            true
        }

        QQC2.Label {
            text:             root.text
            wrapMode:         Text.WordWrap
            Layout.fillWidth: true
        }

        Repeater {
            model: root.actions
            delegate: QQC2.Button {
                flat:      false
                text:      modelData.text
                icon.name: modelData.icon ? (modelData.icon.name ?? "") : ""
                onClicked: modelData.trigger()
            }
        }
    }
}
