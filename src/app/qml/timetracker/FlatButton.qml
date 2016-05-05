import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

Button {
    id: control

    property color textColor: Material.primaryTextColor

    contentItem: Text {
        text: control.text
        font: control.font
        color: !control.enabled ? control.Material.hintTextColor : control.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    Material.background: "transparent"
}
