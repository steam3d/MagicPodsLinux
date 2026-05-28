// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15

Text {
    id: root
    property int level: 1
    color: palette.windowText
    font.bold: true
    font.pointSize: {
        const base = Qt.application.font.pointSize
        switch (level) {
            case 1:  return base * 2.0
            case 2:  return base * 1.5
            case 3:  return base * 1.2
            case 4:  return base * 1.0
            default: return base * 0.9
        }
    }
}
