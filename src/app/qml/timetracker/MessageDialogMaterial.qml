import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0

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
