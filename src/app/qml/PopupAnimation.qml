// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2.15 as QQC2
import magicpods as MP

QQC2.ApplicationWindow {
    id: osdDialog

    property var animationData: ({})
    property var currentDevice: animationData && Object.keys(animationData).length > 0 ? animationData : null
    property bool animationShown: false
    property int animationMode: 0
    property real targetX: 0
    property real targetY: 0
    property real panelStartX: 0
    property real panelStartY: 0
    property real panelStartOpacity: 0.0
    property bool useOpacity: animationMode === 0
    property bool isClosing: false
    property int showRetryCount: 0
    readonly property int margin: 0
    flags: gameScopeMode
        ? (Qt.Popup | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        : (Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
    color: "transparent"
    width: frame.implicitWidth
    height: frame.implicitHeight
    visible: false

    function showOsd() {
        if (width <= 0 || height <= 0 || frame.width <= 0 || frame.height <= 0) {
            if (showRetryCount >= 5) {
                return;
            }
            showRetryCount += 1;
            Qt.callLater(showOsd);
            return;
        }

        showRetryCount = 0;
        closeAnimation.stop();
        computeTargets();
        x = targetX;
        y = targetY;
        frame.x = panelStartX;
        frame.y = panelStartY;
        frame.opacity = panelStartOpacity;
        isClosing = false;
        visible = true;
        osdHide.restart();
        animationShown = true;
        Qt.callLater(function() {
            if (visible) {
                showAnimation.start();
            }
        });
    }

    function hideOsd() {
        requestClose();
    }

    function requestClose() {
        if (!visible || isClosing) {
            return;
        }

        isClosing = true;
        osdHide.stop();
        showAnimation.stop();
        closeAnimation.start();
    }

    function finishClose() {
        closeAnimation.stop();
        showAnimation.stop();
        visible = false;
        animationShown = false;
        isClosing = false;
        animationData = ({});
    }

    function computeTargets() {
        var screenGeo = null;

        if (screen && screen.availableGeometry
                && screen.availableGeometry.width > 0
                && screen.availableGeometry.height > 0) {
            screenGeo = screen.availableGeometry;
        } else if (typeof Screen !== "undefined") {
            screenGeo = Qt.rect(Screen.virtualX, Screen.virtualY, Screen.width, Screen.height);
        }

        if (!screenGeo || screenGeo.width <= 0 || screenGeo.height <= 0) {
            return;
        }

        var left = screenGeo.x + margin;
        var right = screenGeo.x + screenGeo.width - width - margin;
        var top = screenGeo.y + margin;
        var bottom = screenGeo.y + screenGeo.height - height - margin;

        switch (animationMode) {
        case 1:
            targetX = left;
            targetY = bottom;
            panelStartX = -frame.width;
            panelStartY = 0;
            break;
        case 2:
            targetX = right;
            targetY = top;
            panelStartX = frame.width;
            panelStartY = 0;
            break;
        case 3:
            targetX = right;
            targetY = bottom;
            panelStartX = frame.width;
            panelStartY = 0;
            break;
        case 4:
            targetX = left;
            targetY = top;
            panelStartX = -frame.width;
            panelStartY = 0;
            break;
        default:
            targetX = screenGeo.x + Math.round((screenGeo.width - width) / 2);
            targetY = screenGeo.y + Math.round((screenGeo.height - height) / 2);
            panelStartX = 0;
            panelStartY = 0;
            break;
        }

        panelStartOpacity = useOpacity ? 0.0 : 1.0;
    }

    onWidthChanged: {
        if (visible && !isClosing) {
            computeTargets();
            x = targetX;
        }
    }

    onHeightChanged: {
        if (visible && !isClosing) {
            computeTargets();
            y = targetY;
        }
    }

    onClosing: function(event) {
        if (!isClosing) {
            event.accepted = false;
            requestClose();
        }
    }

    Connections {
        target: cppBackend
        enabled: !!cppBackend

        function onDataReceived(json) {
            if (!json || Object.keys(json).length === 0) {
                return;
            }

            if (!json.animation) {
                return;
            }

            if (Object.keys(osdDialog.animationData).length > 0) {
                if (json.animation.address === osdDialog.animationData.address) {
                    osdDialog.animationData = json.animation;
                }
            } else {
                osdDialog.animationData = json.animation;
            }

            if (typeof json.animation.show === "undefined") {
                return;
            }

            if (json.animation.show === true && !osdDialog.animationShown) {
                osdDialog.showOsd();
                return;
            }
            if (json.animation.show === false && osdDialog.animationShown) {
                osdDialog.hideOsd();
            }
        }
    }

    Timer {
        id: osdHide
        interval: 30000
        repeat: false
        onTriggered: osdDialog.requestClose()
    }

    QQC2.Pane {
        id: frame
        width: osdDialog.width
        height: osdDialog.height
        implicitWidth: popupRoot.width + (MP.Units.smallSpacing * 2)
        implicitHeight: popupRoot.height + (MP.Units.smallSpacing * 2)
        padding: 0
        background: Rectangle {
            radius: 14
            color: frame.palette.window
            border.width: 1
            border.color: Qt.rgba(frame.palette.windowText.r, frame.palette.windowText.g, frame.palette.windowText.b, 0.15)
        }

        Item {
            id: popupRoot
            anchors.centerIn: parent
            width: 228
            height: isExt ? 238 + 18 : 238
            clip: true

            MP.Animations {
                id: animStore
            }

            property var device: osdDialog.currentDevice
            property bool connected: (device && typeof device.connected === "boolean") ? device.connected : false
            property string deviceAddress: (device && typeof device.address === "string") ? device.address : ""
            property var batteryInfo: device && device.battery ? device.battery : null
            property var batteryLeft: batteryInfo ? batteryInfo.left : null
            property var batteryRight: batteryInfo ? batteryInfo.right : null
            property var batteryCase: batteryInfo ? batteryInfo.case : null
            property var batterySingle: batteryInfo ? batteryInfo.single : null
            property bool useSingle: batterySingle && batterySingle.status !== 0 && (!batteryLeft || batteryLeft.status === 0) && (!batteryRight || batteryRight.status === 0) && (!batteryCase || batteryCase.status === 0)
            property string resolvedKey: {
                if (!device) {
                    return animStore.fallbackKey;
                }
                var vendor = device?.vendor ?? 0;
                var model = device?.model ?? 0;
                var color = device?.color ?? 0;
                var fullKey = vendor + "_" + model + "_" + color;
                var shortKey = vendor + "_" + model;

                if (animStore.animations && animStore.animations[fullKey]) {
                    return fullKey;
                }
                if (animStore.animations && animStore.animations[shortKey]) {
                    return shortKey;
                }
                return animStore.fallbackKey;
            }
            property var selectedAnim: animStore.get(resolvedKey)
            readonly property int seqWidth: selectedAnim ? selectedAnim.frameWidth : 0
            readonly property int seqHeight: selectedAnim ? selectedAnim.frameHeight : 0
            readonly property bool isExt: seqWidth === 200 && seqHeight === 132
        }

        ColumnLayout {
            anchors.fill: popupRoot
            spacing: 0

            QQC2.ToolButton {
                id: closeButton
                text: "×"
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                onClicked: osdDialog.hideOsd()
            }

            ColumnLayout {
                Layout.topMargin: -8
                Layout.maximumWidth: 180
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                spacing: 0

                MP.Heading {
                    Layout.alignment: Qt.AlignHCenter
                    level: 1
                    font.bold: false
                    text: popupRoot.device && popupRoot.device.name ? popupRoot.device.name : ""
                    wrapMode: Text.WordWrap
                    font.pointSize: Qt.application.font.pointSize * 1.2
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    maximumLineCount: 1
                }

                SpriteSequence {
                    id: sequenceAnim
                    Layout.topMargin: 8
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 180
                    Layout.preferredHeight: popupRoot.isExt ? 108 : 90
                    running: osdDialog.visible
                    width: 180
                    height: Layout.preferredHeight

                    Sprite {
                        source: `qrc:/qt/qml/magicpods/src/app/qml/assets/animations/${popupRoot.selectedAnim.source}`
                        frameWidth: popupRoot.selectedAnim.frameWidth
                        frameHeight: popupRoot.selectedAnim.frameHeight
                        frameCount: popupRoot.selectedAnim.frameCount
                        frameRate: popupRoot.selectedAnim.frameRate
                    }
                }

                RowLayout {
                    Layout.topMargin: 8
                    Layout.fillWidth: true
                    visible: !popupRoot.useSingle

                    MP.BatteryPopup {
                        Layout.leftMargin: 8
                        battery: popupRoot.batteryLeft ? popupRoot.batteryLeft.battery : 0
                        isCharging: popupRoot.batteryLeft ? popupRoot.batteryLeft.charging : false
                        status: popupRoot.batteryLeft ? popupRoot.batteryLeft.status : 0
                    }

                    MP.BatteryPopup {
                        Layout.leftMargin: 4
                        battery: popupRoot.batteryRight ? popupRoot.batteryRight.battery : 0
                        isCharging: popupRoot.batteryRight ? popupRoot.batteryRight.charging : false
                        status: popupRoot.batteryRight ? popupRoot.batteryRight.status : 0
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    MP.BatteryPopup {
                        Layout.rightMargin: 24
                        battery: popupRoot.batteryCase ? popupRoot.batteryCase.battery : 0
                        isCharging: popupRoot.batteryCase ? popupRoot.batteryCase.charging : false
                        status: popupRoot.batteryCase ? popupRoot.batteryCase.status : 0
                    }
                }

                MP.BatteryPopup {
                    Layout.topMargin: 8
                    battery: popupRoot.batterySingle ? popupRoot.batterySingle.battery : 0
                    isCharging: popupRoot.batterySingle ? popupRoot.batterySingle.charging : false
                    status: popupRoot.batterySingle ? popupRoot.batterySingle.status : 0
                    Layout.alignment: Qt.AlignHCenter
                    visible: popupRoot.useSingle
                }

                QQC2.Button {
                    Layout.topMargin: 16
                    text: popupRoot.connected ? qsTrId("tray.disconnect") : qsTrId("tray.connect")
                    Layout.fillWidth: true
                    onClicked: {
                        if (!popupRoot.deviceAddress || !cppBackend) {
                            return;
                        }
                        if (popupRoot.connected) {
                            cppBackend.disconnectDevice(popupRoot.deviceAddress);
                        } else {
                            cppBackend.connectDevice(popupRoot.deviceAddress);
                        }
                        osdDialog.hideOsd();
                    }
                }
            }
        }
    }

    ParallelAnimation {
        id: showAnimation
        running: false

        NumberAnimation {
            target: frame
            property: "x"
            to: 0
            duration: 350
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: frame
            property: "y"
            to: 0
            duration: 350
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: frame
            property: "opacity"
            to: 1.0
            duration: useOpacity ? 350 : 0
            easing.type: Easing.OutCubic
        }
    }

    ParallelAnimation {
        id: closeAnimation
        running: false

        NumberAnimation {
            target: frame
            property: "x"
            to: panelStartX
            duration: 350
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: frame
            property: "y"
            to: panelStartY
            duration: 350
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: frame
            property: "opacity"
            to: panelStartOpacity
            duration: useOpacity ? 350 : 0
            easing.type: Easing.OutCubic
        }

        onStopped: {
            if (isClosing) {
                finishClose();
            }
        }
    }
}
