// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import magicpods as MP
import "../components" as Components

QQC2.Page {
    id: rootPage
    padding: 0
    background: Rectangle {
        color: "transparent"
    }

    property var infoData: ({})
    readonly property bool hasInfo: infoData && Object.keys(infoData).length > 0
    readonly property bool backendConnected: !!cppBackend && cppBackend.connected
    property int selectedAnc: ancData?.selected ?? 0
    readonly property var ancModes: ({
            OFF: 1,
            TRANSPARENCY: 2,
            ADAPTIVE: 4,
            WIND: 8,
            ANC: 16
        })
    property var data: infoData
    readonly property var capabilities: data?.capabilities ?? null
    readonly property var ancData: capabilities?.anc ?? null
    readonly property var conversationAwarenessData: capabilities?.conversationAwareness ?? null
    readonly property var personalizedVolumeData: capabilities?.personalizedVolume ?? null
    readonly property var ancOneAirPodData: capabilities?.ancOneAirPod ?? null
    readonly property var volumeSwipeData: capabilities?.volumeSwipe ?? null
    readonly property var adaptiveAudioNoiseData: capabilities?.adaptiveAudioNoise ?? null
    readonly property var pressAndHoldDurationData: capabilities?.pressAndHoldDuration ?? null
    readonly property var pressSpeedData: capabilities?.pressSpeed ?? null
    readonly property var toneVolumeData: capabilities?.toneVolume ?? null
    readonly property var volumeSwipeLengthData: capabilities?.volumeSwipeLength ?? null
    readonly property var endCallData: capabilities?.endCall ?? null
    readonly property var bluetoothCodec: capabilities?.bluetoothCodec ?? null
    readonly property int mWidth: MP.Units.gridUnit * 12

    readonly property bool hasCapabilities: !!capabilities && [ancData, conversationAwarenessData, personalizedVolumeData, ancOneAirPodData, volumeSwipeData, adaptiveAudioNoiseData, pressAndHoldDurationData, pressSpeedData, toneVolumeData, volumeSwipeLengthData, endCallData, bluetoothCodec].some(function (v) {
        return v !== null;
    })

    title: qsTrId("menu.battery")

    function assetPath(relativePath) {
        return "qrc:/qt/qml/magicpods/src/app/qml/assets/" + relativePath;
    }

    function currentAddress() {
        return infoData?.address;
    }

    function requestInfo() {
        if (cppBackend && cppBackend.connected) {
            cppBackend.getInfo();
        }
    }

    Connections {
        target: cppBackend
        enabled: !!cppBackend
        function onDataReceived(json) {
            if (!json || Object.keys(json).length === 0) {
                rootPage.infoData = ({});
            } else if (json.info) {
                rootPage.infoData = json.info;
                rootPage.selectedAnc = json.info?.capabilities?.anc?.selected ?? rootPage.selectedAnc;
            }
        }
        function onConnectedChanged() {
            requestInfo();
        }
    }
    Component.onCompleted: {
        requestInfo();
    }

    Components.HelpMessage {
        visible: !hasInfo
        iconSource: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/menu-headphones.png"
        titleText: qsTrId("battery.connect_headphones.header")
        bodyText: qsTrId("battery.connect_headphones.description")
        width: mWidth * 1.5
    }

    QQC2.ScrollView {
        id: batteryScrollView
        anchors.fill: parent
        visible: hasInfo
        contentWidth: availableWidth        

        ColumnLayout {
            id: contentLayout
            width: batteryScrollView.availableWidth
            y: Math.max(0, (batteryScrollView.availableHeight - implicitHeight) / 2)

            RowLayout {
                Layout.fillWidth: true
                Layout.maximumWidth: mWidth * 1.5
                Layout.minimumWidth: mWidth * 1.5
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: MP.Units.largeSpacing
                spacing: MP.Units.mediumSpacing

                Image {
                    id: headphonesImage
                    property var candidates: []
                    property int candidateIndex: 0
                    Layout.preferredWidth: MP.Units.iconSizes.huge
                    Layout.preferredHeight: MP.Units.iconSizes.huge
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                    source: ""

                    function buildCandidates() {
                        var v = infoData?.vendor ?? 0;
                        var m = infoData?.model ?? 0;
                        var c = infoData?.color ?? 0;
                        var basePath = "headphones/" + v + "_";
                        var next = [];
                        next.push(rootPage.assetPath(basePath + m + "_" + c + ".png"));
                        next.push(rootPage.assetPath(basePath + m + ".png"));
                        next.push(rootPage.assetPath("headphones/0_0_0.png"));
                        candidates = next;
                        candidateIndex = 0;
                        tryNext();
                    }

                    function tryNext() {
                        if (candidateIndex >= candidates.length) {
                            return;
                        }
                        source = candidates[candidateIndex];
                    }

                    onStatusChanged: {
                        if (status === Image.Error) {
                            candidateIndex += 1;
                            tryNext();
                        }
                    }

                    Component.onCompleted: headphonesImage.buildCandidates()
                    Connections {
                        target: rootPage
                        function onInfoDataChanged() {
                            headphonesImage.buildCandidates();
                        }
                    }
                }

                ColumnLayout {
                    MP.Heading {
                        level: 2
                        text: rootPage.infoData?.name ?? ""
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        maximumLineCount: 1
                    }

                    RowLayout {
                        Layout.maximumWidth: mWidth * 1.5
                        spacing: MP.Units.mediumSpacing
                        Layout.fillWidth: false

                        Components.BatteryVertical {
                            name: qsTrId("battery.battery_single")
                            battery: rootPage.capabilities?.battery?.single?.battery ?? 0
                            isCharging: rootPage.capabilities?.battery?.single?.charging ?? false
                            status: rootPage.capabilities?.battery?.single?.status ?? 0
                        }

                        Components.BatteryVertical {
                            name: qsTrId("battery.battery_left")
                            battery: rootPage.capabilities?.battery?.left?.battery ?? 0
                            isCharging: rootPage.capabilities?.battery?.left?.charging ?? false
                            status: rootPage.capabilities?.battery?.left?.status ?? 0
                        }

                        Components.BatteryVertical {
                            name: qsTrId("battery.battery_right")
                            battery: rootPage.capabilities?.battery?.right?.battery ?? 0
                            isCharging: rootPage.capabilities?.battery?.right?.charging ?? false
                            status: rootPage.capabilities?.battery?.right?.status ?? 0
                        }

                        Components.BatteryVertical {
                            name: qsTrId("battery.battery_case")
                            battery: rootPage.capabilities?.battery?.case?.battery ?? 0
                            isCharging: rootPage.capabilities?.battery?.case?.charging ?? false
                            status: rootPage.capabilities?.battery?.case?.status ?? 0
                        }
                    }
                }
            }

            RowLayout {
                Layout.maximumWidth: mWidth * 1.5
                Layout.topMargin: MP.Units.mediumSpacing
                spacing: MP.Units.mediumSpacing
                visible: rootPage.ancData !== null
                Layout.alignment: Qt.AlignHCenter

                QQC2.ButtonGroup { id: ancButtonGroup }

                QQC2.Button {
                    Layout.preferredHeight: 32
                    padding: 4
                    Layout.fillWidth: true
                    checkable: true
                    QQC2.ButtonGroup.group: ancButtonGroup
                    checked: rootPage.selectedAnc === rootPage.ancModes.OFF
                    visible: (rootPage.ancData?.options ?? 0) & rootPage.ancModes.OFF
                    enabled: !(rootPage.ancData?.readonly ?? true)
                    display: QQC2.AbstractButton.IconOnly
                    icon.source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/icon-off.svg"
                    icon.color: checked ? palette.highlightedText : palette.buttonText
                    onClicked: if (rootPage.ancData) {
                        rootPage.selectedAnc = rootPage.ancModes.OFF;
                        rootPage.ancData.selected = rootPage.selectedAnc;
                        cppBackend.setAnc(rootPage.currentAddress(), rootPage.selectedAnc);
                    }
                    QQC2.ToolTip {
                        visible: parent.hovered
                        delay: 500
                        text: qsTrId("battery.anc_off")
                    }
                }

                QQC2.Button {
                    Layout.preferredHeight: 32
                    padding: 4
                    Layout.fillWidth: true
                    checkable: true
                    QQC2.ButtonGroup.group: ancButtonGroup
                    checked: rootPage.selectedAnc === rootPage.ancModes.TRANSPARENCY
                    visible: (rootPage.ancData?.options ?? 0) & rootPage.ancModes.TRANSPARENCY
                    enabled: !(rootPage.ancData?.readonly ?? true)
                    display: QQC2.AbstractButton.IconOnly
                    icon.source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/icon-tra.svg"
                    icon.color: checked ? palette.highlightedText : palette.buttonText
                    onClicked: if (rootPage.ancData) {
                        rootPage.selectedAnc = rootPage.ancModes.TRANSPARENCY;
                        rootPage.ancData.selected = rootPage.selectedAnc;
                        cppBackend.setAnc(rootPage.currentAddress(), rootPage.selectedAnc);
                    }
                    QQC2.ToolTip {
                        visible: parent.hovered
                        delay: 500
                        text: qsTrId("battery.anc_tra")
                    }
                }

                QQC2.Button {
                    Layout.preferredHeight: 32
                    padding: 4
                    Layout.fillWidth: true
                    checkable: true
                    QQC2.ButtonGroup.group: ancButtonGroup
                    checked: rootPage.selectedAnc === rootPage.ancModes.ADAPTIVE
                    visible: (rootPage.ancData?.options ?? 0) & rootPage.ancModes.ADAPTIVE
                    enabled: !(rootPage.ancData?.readonly ?? true)
                    display: QQC2.AbstractButton.IconOnly
                    icon.source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/icon-adaptive.svg"
                    icon.color: checked ? palette.highlightedText : palette.buttonText
                    onClicked: if (rootPage.ancData) {
                        rootPage.selectedAnc = rootPage.ancModes.ADAPTIVE;
                        rootPage.ancData.selected = rootPage.selectedAnc;
                        cppBackend.setAnc(rootPage.currentAddress(), rootPage.selectedAnc);
                    }
                    QQC2.ToolTip {
                        visible: parent.hovered
                        delay: 500
                        text: qsTrId("battery.anc_adaptive")
                    }
                }

                QQC2.Button {
                    Layout.preferredHeight: 32
                    padding: 4
                    Layout.fillWidth: true
                    checkable: true
                    QQC2.ButtonGroup.group: ancButtonGroup
                    checked: rootPage.selectedAnc === rootPage.ancModes.WIND
                    visible: (rootPage.ancData?.options ?? 0) & rootPage.ancModes.WIND
                    enabled: !(rootPage.ancData?.readonly ?? true)
                    display: QQC2.AbstractButton.IconOnly
                    icon.source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/icon-wind.svg"
                    icon.color: checked ? palette.highlightedText : palette.buttonText
                    onClicked: if (rootPage.ancData) {
                        rootPage.selectedAnc = rootPage.ancModes.WIND;
                        rootPage.ancData.selected = rootPage.selectedAnc;
                        cppBackend.setAnc(rootPage.currentAddress(), rootPage.selectedAnc);
                    }
                    QQC2.ToolTip {
                        visible: parent.hovered
                        delay: 500
                        text: qsTrId("battery.anc_wind")
                    }
                }

                QQC2.Button {
                    Layout.preferredHeight: 32
                    padding: 4
                    Layout.fillWidth: true
                    checkable: true
                    QQC2.ButtonGroup.group: ancButtonGroup
                    checked: rootPage.selectedAnc === rootPage.ancModes.ANC
                    visible: (rootPage.ancData?.options ?? 0) & rootPage.ancModes.ANC
                    enabled: !(rootPage.ancData?.readonly ?? true)
                    display: QQC2.AbstractButton.IconOnly
                    icon.source: "qrc:/qt/qml/magicpods/src/app/qml/assets/icons/icon-noise.svg"
                    icon.color: checked ? palette.highlightedText : palette.buttonText
                    onClicked: if (rootPage.ancData) {
                        rootPage.selectedAnc = rootPage.ancModes.ANC;
                        rootPage.ancData.selected = rootPage.selectedAnc;
                        cppBackend.setAnc(rootPage.currentAddress(), rootPage.selectedAnc);
                    }
                    QQC2.ToolTip {
                        visible: parent.hovered
                        delay: 500
                        text: qsTrId("battery.anc_anc")
                    }
                }
            }

            MP.Separator {
                Layout.maximumWidth: mWidth * 1.5
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: MP.Units.smallSpacing
            }

            ColumnLayout {
                Layout.fillWidth: true
                visible: hasCapabilities
                spacing: MP.Units.mediumSpacing
                Layout.topMargin: MP.Units.largeSpacing

                MP.Label {
                    Layout.fillWidth: true
                    Layout.bottomMargin: MP.Units.smallSpacing
                    font.bold: true
                    text: qsTrId("battery.capabilities.header")
                    horizontalAlignment: Text.AlignHCenter
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.bluetoothCodec !== null
                    label: qsTrId("battery.bluetooth_codec")
                    tooltip: {
                        var options = rootPage.bluetoothCodec?.options || [];
                        var lines = [];
                        options.forEach(function (option) {
                            if (!option || option.length < 2) return;
                            lines.push(String(option[0]) + " — " + String(option[1]));
                        });
                        return lines.join("\n\n");
                    }

                    QQC2.ComboBox {
                        width: rootPage.mWidth
                        model: (rootPage.bluetoothCodec?.options ?? []).map(function (option) {
                            return option[0];
                        })
                        currentIndex: {
                            var options = rootPage.bluetoothCodec?.options || [];
                            for (var i = 0; i < options.length; i++) {
                                if (options[i][0] === rootPage.bluetoothCodec.selected) return i;
                            }
                            return -1;
                        }
                        enabled: !(rootPage.bluetoothCodec?.readonly ?? true)
                        onActivated: {
                            if (rootPage.bluetoothCodec && currentIndex >= 0) {
                                var options = rootPage.bluetoothCodec.options || [];
                                if (options[currentIndex]) {
                                    var nextValue = options[currentIndex][0];
                                    if (rootPage.bluetoothCodec.selected !== nextValue) {
                                        rootPage.bluetoothCodec.selected = nextValue;
                                        cppBackend.setCapability("bluetoothCodec", rootPage.currentAddress(), nextValue);
                                    }
                                }
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.conversationAwarenessData !== null
                    label: qsTrId("battery.conversation_awareness")

                    QQC2.Switch {
                        checked: rootPage.conversationAwarenessData?.selected ?? false
                        enabled: !(rootPage.conversationAwarenessData?.readonly ?? true)
                        onToggled: {
                            if (rootPage.conversationAwarenessData) {
                                rootPage.conversationAwarenessData.selected = checked;
                                cppBackend.setCapability("conversationAwareness", rootPage.currentAddress(), checked);
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.personalizedVolumeData !== null
                    label: qsTrId("battery.personalized_volume")

                    QQC2.Switch {
                        checked: rootPage.personalizedVolumeData?.selected ?? false
                        enabled: !(rootPage.personalizedVolumeData?.readonly ?? true)
                        onToggled: {
                            if (rootPage.personalizedVolumeData) {
                                rootPage.personalizedVolumeData.selected = checked;
                                cppBackend.setCapability("personalizedVolume", rootPage.currentAddress(), checked);
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.adaptiveAudioNoiseData !== null
                    label: qsTrId("battery.adaptive_audio_noise")

                    QQC2.ComboBox {
                        width: rootPage.mWidth
                        model: [qsTrId("battery.adaptive_audio_noise.more"), qsTrId("battery.adaptive_audio_noise.default"), qsTrId("battery.adaptive_audio_noise.less")]
                        currentIndex: {
                            const val = rootPage.adaptiveAudioNoiseData?.selected;
                            if (val === 0) return 0;
                            if (val === 50) return 1;
                            if (val === 100) return 2;
                            return 1;
                        }
                        enabled: !(rootPage.adaptiveAudioNoiseData?.readonly ?? true)
                        onActivated: {
                            if (rootPage.adaptiveAudioNoiseData) {
                                if (currentIndex === 0) {
                                    rootPage.adaptiveAudioNoiseData.selected = 0;
                                } else if (currentIndex === 1) {
                                    rootPage.adaptiveAudioNoiseData.selected = 50;
                                } else {
                                    rootPage.adaptiveAudioNoiseData.selected = 100;
                                }
                                cppBackend.setCapability("adaptiveAudioNoise", rootPage.currentAddress(), rootPage.adaptiveAudioNoiseData.selected);
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.ancOneAirPodData !== null
                    label: qsTrId("battery.anc_one_airpod")

                    QQC2.Switch {                        
                        checked: rootPage.ancOneAirPodData?.selected ?? false
                        enabled: !(rootPage.ancOneAirPodData?.readonly ?? true)
                        onToggled: {
                            if (rootPage.ancOneAirPodData) {
                                rootPage.ancOneAirPodData.selected = checked;
                                cppBackend.setCapability("ancOneAirPod", rootPage.currentAddress(), checked);
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.pressAndHoldDurationData !== null
                    label: qsTrId("battery.press_and_hold_duration")

                    QQC2.ComboBox {
                        width: rootPage.mWidth
                        model: [qsTrId("battery.press_and_hold_duration.default"), qsTrId("battery.press_and_hold_duration.shorter"), qsTrId("battery.press_and_hold_duration.shortest")]
                        currentIndex: rootPage.pressAndHoldDurationData?.selected ?? 0
                        enabled: !(rootPage.pressAndHoldDurationData?.readonly ?? true)
                        onActivated: {
                            if (rootPage.pressAndHoldDurationData) {
                                rootPage.pressAndHoldDurationData.selected = currentIndex;
                                cppBackend.setCapability("pressAndHoldDuration", rootPage.currentAddress(), currentIndex);
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.pressSpeedData !== null
                    label: qsTrId("battery.press_speed")

                    QQC2.ComboBox {
                        width: rootPage.mWidth
                        model: [qsTrId("battery.press_speed.default"), qsTrId("battery.press_speed.slower"), qsTrId("battery.press_speed.slowest")]
                        currentIndex: rootPage.pressSpeedData?.selected ?? 0
                        enabled: !(rootPage.pressSpeedData?.readonly ?? true)
                        onActivated: {
                            if (rootPage.pressSpeedData) {
                                rootPage.pressSpeedData.selected = currentIndex;
                                cppBackend.setCapability("pressSpeed", rootPage.currentAddress(), currentIndex);
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.toneVolumeData !== null
                    label: qsTrId("battery.tone_volume")

                    RowLayout {
                        spacing: MP.Units.largeSpacing

                        QQC2.Slider {
                            id: toneVolumeSlider
                            Layout.leftMargin: -16
                            Layout.rightMargin: -16
                            implicitWidth: rootPage.mWidth + 32
                            from: 15
                            to: 125
                            value: rootPage.toneVolumeData?.selected ?? 50
                            enabled: !(rootPage.toneVolumeData?.readonly ?? true)
                            onMoved: {
                                if (rootPage.toneVolumeData) {
                                    rootPage.toneVolumeData.selected = Math.round(value);
                                }
                            }
                            onPressedChanged: {
                                if (!pressed && rootPage.toneVolumeData) {
                                    cppBackend.setCapability("toneVolume", rootPage.currentAddress(), rootPage.toneVolumeData.selected);
                                }
                            }
                        }

                        MP.Label {
                            text: Math.round(toneVolumeSlider.value) + "%"
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.volumeSwipeData !== null
                    label: qsTrId("battery.volume_swipe")

                    QQC2.Switch {
                        id: volumeSwipe
                        checked: rootPage.volumeSwipeData?.selected ?? false
                        enabled: !(rootPage.volumeSwipeData?.readonly ?? true)
                        onToggled: {
                            if (rootPage.volumeSwipeData) {
                                rootPage.volumeSwipeData.selected = checked;
                                cppBackend.setCapability("volumeSwipe", rootPage.currentAddress(), checked);
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.volumeSwipeLengthData !== null
                    label: qsTrId("battery.volume_swipe_length")

                    QQC2.ComboBox {
                        width: rootPage.mWidth
                        model: [qsTrId("battery.volume_swipe_length.default"), qsTrId("battery.volume_swipe_length.longer"), qsTrId("battery.volume_swipe_length.longest")]
                        currentIndex: rootPage.volumeSwipeLengthData?.selected ?? 0
                        enabled: (!(rootPage.volumeSwipeLengthData?.readonly ?? true) && volumeSwipe.checked)
                        onActivated: {
                            if (rootPage.volumeSwipeLengthData) {
                                rootPage.volumeSwipeLengthData.selected = currentIndex;
                                cppBackend.setCapability("volumeSwipeLength", rootPage.currentAddress(), currentIndex);
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.endCallData !== null
                    label: qsTrId("battery.end_call")

                    QQC2.ComboBox {
                        width: rootPage.mWidth
                        model: [qsTrId("battery.end_call.twice"), qsTrId("battery.end_call.once")]
                        currentIndex: (rootPage.endCallData?.selected === 3) ? 1 : 0
                        enabled: !(rootPage.endCallData?.readonly ?? true)
                        onActivated: {
                            if (rootPage.endCallData) {
                                rootPage.endCallData.selected = currentIndex === 0 ? 2 : 3;
                                cppBackend.setCapability("endCall", rootPage.currentAddress(), rootPage.endCallData.selected);
                            }
                        }
                    }
                }

                MP.FormRow {
                    Layout.fillWidth: true
                    visible: rootPage.endCallData !== null
                    label: qsTrId("battery.mute_unmute")

                    QQC2.ComboBox {
                        width: rootPage.mWidth
                        model: [qsTrId("battery.end_call.twice"), qsTrId("battery.end_call.once")]
                        currentIndex: (rootPage.endCallData?.selected === 2) ? 1 : 0
                        enabled: false
                    }
                }
            }
            Item {
                Layout.preferredHeight: 0
            }
        }
    }
}
