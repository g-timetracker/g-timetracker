import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Material.impl 2.0

Button {
    id: control

    property url iconSource

    contentItem: Image {
        fillMode: Image.Pad
        source: iconSource
    }

    Material.elevation: control.down ? 12 : 6

    background: Rectangle {
        implicitHeight: 56
        implicitWidth: 56
        radius: height / 2
        color: control.down ? control.Material.highlightedButtonPressColor
                            : control.visualFocus ? control.Material.highlightedButtonHoverColor
                                                  : control.Material.highlightedButtonColor

        layer.enabled: control.enabled
        layer.effect: ElevationEffect {
            elevation: control.Material.elevation
        }
    }
}
