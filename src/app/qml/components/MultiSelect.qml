// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

Item {
    id: root

    property string title: ""
    property string hint: ""
    property string doNotApplyLabel: ""
    property int value: -1
    property int doNotApplyValue: -1
    property int minimumSelected: 2
    property var options: []

    signal valueCommitted(int value)

    property int _displayValue: value
    property int draftValue: 0
    readonly property int availableValues: {
        var result = 0;
        for (var i = 0; i < options.length; ++i) {
            result |= options[i].value;
        }
        return result;
    }
    readonly property var selectedOptions: {
        if (_displayValue === doNotApplyValue) {
            return [];
        }
        return options.filter(function (option) {
            return (_displayValue & option.value) !== 0;
        });
    }

    implicitWidth: selectButton.implicitWidth
    implicitHeight: selectButton.implicitHeight

    onValueChanged: _displayValue = value

    function selectedCount(selectedValue) {
        return options.filter(function (option) {
            return (selectedValue & option.value) !== 0;
        }).length;
    }

    function isValid(selectedValue) {
        const count = selectedCount(selectedValue);
        return count === 0 || count >= minimumSelected;
    }

    function setOption(optionValue, checked) {
        draftValue = (checked
            ? draftValue | optionValue
            : draftValue & ~optionValue) & availableValues;
        selectDialog.updateOkButton();
    }

    QQC2.Button {
        id: selectButton
        anchors.fill: parent
        onClicked: selectDialog.open()

        contentItem: Item {
            implicitWidth: buttonContent.implicitWidth
            implicitHeight: buttonContent.implicitHeight

            RowLayout {
                id: buttonContent
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                QQC2.Label {
                    visible: root.selectedOptions.length === 0
                    text: root.doNotApplyLabel
                }

                Repeater {
                    model: root.selectedOptions

                    delegate: Image {
                        required property var modelData
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: 20
                        source: modelData.icon
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
        }
    }

    QQC2.Dialog {
        id: selectDialog
        parent: QQC2.Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(360, parent ? parent.width - 32 : 360)
        modal: true
        focus: true
        title: root.title
        standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel

        header: QQC2.Label {
            visible: text !== ""
            text: selectDialog.title
            font.bold: true
            font.pixelSize: 18
            padding: 12
            wrapMode: Text.WordWrap
        }

        function updateOkButton() {
            const button = standardButton(QQC2.Dialog.Ok);
            if (button) {
                button.enabled = root.isValid(root.draftValue);
            }
        }

        onAboutToShow: {
            root.draftValue = root._displayValue === root.doNotApplyValue
                ? 0
                : root._displayValue & root.availableValues;
        }
        onOpened: updateOkButton()
        onAccepted: {
            const selectedValue = root.draftValue === 0
                ? root.doNotApplyValue
                : root.draftValue;
            root._displayValue = selectedValue;
            root.valueCommitted(selectedValue);
        }

        contentItem: ColumnLayout {
            spacing: 8

            QQC2.Label {
                Layout.fillWidth: true
                visible: root.hint !== ""
                text: root.hint
                opacity: 0.7
                wrapMode: Text.WordWrap
            }

            Repeater {
                model: root.options

                delegate: QQC2.CheckBox {
                    id: optionCheckBox
                    required property var modelData
                    Layout.fillWidth: true
                    text: modelData.label
                    checked: (root.draftValue & modelData.value) !== 0
                    onClicked: root.setOption(modelData.value, checked)

                    contentItem: RowLayout {
                        spacing: 8

                        Item {
                            Layout.preferredWidth: optionCheckBox.indicator
                                ? optionCheckBox.indicator.width + optionCheckBox.spacing
                                : 0
                        }

                        Image {
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            source: optionCheckBox.modelData.icon
                            fillMode: Image.PreserveAspectFit
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: optionCheckBox.modelData.label
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }
}
