// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15
import QtQuick.Layouts 1.15
import magicpods as MP

ColumnLayout {
    required property url iconSource
    required property string titleText
    required property string bodyText
    anchors.centerIn: parent

    Image {
        Layout.preferredWidth: 72
        Layout.preferredHeight: 72
        fillMode: Image.PreserveAspectFit
        source: iconSource
        Layout.alignment: Qt.AlignHCenter
        smooth: true
        mipmap: true
    }

    MP.Heading {
        text: titleText
        level: 2
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        elide: Text.ElideRight
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Text.AlignHCenter
    }

    MP.Label {
        text: bodyText
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
