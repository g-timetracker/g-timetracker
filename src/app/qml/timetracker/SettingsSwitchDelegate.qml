import QtQuick 2.4
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0

SettingsDelegate {
    id: control

    indicator: Switch {
        anchors.margins: control.padding
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        enabled: control.enabled

        checked: control.checked
        onCheckedChanged: control.checked = checked
    }

    label: Text {
        anchors.left: parent.left
        anchors.right: indicator.right
        anchors.leftMargin: control.padding
        anchors.rightMargin: control.spacing
        anchors.verticalCenter: parent.verticalCenter
        height: control.availableHeight
        text: control.text
        font: control.font
        color: control.enabled ? control.Material.primaryTextColor : control.Material.hintTextColor
        elide: Text.ElideRight
        visible: control.text
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}
