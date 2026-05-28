// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

pragma Singleton
import QtQuick

QtObject {
    readonly property int smallSpacing: 4
    readonly property int mediumSpacing: 8
    readonly property int largeSpacing: 16
    readonly property int hugeSpacing: 24
    readonly property int gridUnit: 18

    readonly property var iconSizes: ({ medium: 22, huge: 64 })
}
