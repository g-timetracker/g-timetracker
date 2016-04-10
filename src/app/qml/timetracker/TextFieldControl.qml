import QtQuick 2.4
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0

Column {
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    property string helperText

    spacing: 0

    TextField {
        id: textField

        topPadding: floatingLabel.height + 8
        width: parent.width
        selectByMouse: true

        LabelControl {
            id: floatingLabel

            width: parent.width
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            color: textField.activeFocus ? textField.Material.accentColor : textField.Material.hintTextColor
            text: !!textField.text ? textField.placeholderText : ""
        }
    }

    LabelControl {
        id: helperLabel

        width: parent.width
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        visible: !!helperText
        text: textField.activeFocus ? helperText : ""
    }
}
