import QtQuick 2.4
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0

Column {
    property alias text: textArea.text
    property alias placeholderText: textArea.placeholderText
    property string helperText

    spacing: 0

    TextArea {
        id: textArea

        topPadding: floatingLabel.height + 8
        width: parent.width
        selectByMouse: true

        LabelControl {
            id: floatingLabel

            width: parent.width
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            color: textArea.activeFocus ? textArea.Material.accentColor : textArea.Material.hintTextColor
            text: !!textArea.text ? textArea.placeholderText : ""
        }
    }

    LabelControl {
        id: helperLabel

        width: parent.width
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        visible: !!helperText
        text: textArea.activeFocus ? helperText : ""
    }
}
