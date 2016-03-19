import QtQuick 2.4
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0

SettingsDelegate {
    id: control

    property string additionalText

    height: 72

    label: Column {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: control.padding
        spacing: 4

        Text {
            width: parent.width
            text: control.text
            font: control.font
            color: control.enabled ? control.Material.primaryTextColor : control.Material.hintTextColor
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        Label {
            width: parent.width
            text: control.additionalText
            font.weight: Font.Normal
            font.pixelSize: 14
            color: control.Material.hintTextColor
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }
}
