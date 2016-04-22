import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

DialogMaterial {
    property alias text: label.text

    Label {
        id: label

        leftPadding: 24
        rightPadding: 24
        bottomPadding: 24
        font.pixelSize: 16
        color: Material.secondaryTextColor
    }
}
