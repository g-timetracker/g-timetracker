import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtGraphicalEffects 1.0

Button {
    id: control

    property url iconSource

    contentItem: Image {
        fillMode: Image.Pad
        source: iconSource
    }

    background: Rectangle {
        implicitHeight: 56
        implicitWidth: 56
        radius: height / 2
        color: control.Material.accentColor

        layer.enabled: control.enabled
        layer.effect: DropShadow {
            verticalOffset: control.pressed ? 12 : 6
            radius: control.pressed ? 12 : 6
            color: control.Material.dropShadowColor
            samples: control.pressed ? 24 : 12
            spread: 0.0
        }
    }
}
