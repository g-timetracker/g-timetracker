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

    background: Rectangle {
        implicitWidth: 64
        implicitHeight: 48

        y: 6
        width: parent.width
        height: parent.height - 12
        radius: 2
        color: control.down ? control.Material.flatButtonPressColor
                            : control.visualFocus ? control.Material.flatButtonFocusColor
                                                  : "transparent"

        Behavior on color {
            ColorAnimation {
                duration: 400
            }
        }

        Rectangle {
            width: parent.width
            height: parent.height
            radius: parent.radius
            visible: control.visualFocus
            color: control.Material.checkBoxUncheckedRippleColor
        }
    }
}
