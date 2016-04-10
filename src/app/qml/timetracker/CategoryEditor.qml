import QtQuick 2.4
import Qt.labs.controls 1.0

Item {
    id: editor

    property alias categoryName: nameField.text
    property alias categoryComment: commentArea.text

    property bool acceptable: !!categoryName

    implicitHeight: container.height + container.anchors.margins * 2

    Column {
        id: container

        anchors.margins: 16
        anchors.topMargin: 24
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 8

        TextFieldControl {
            id: nameField

            width: parent.width
            placeholderText: "Name*"
            helperText: "use '>' to split the name into subcategories"
        }

        TextAreaControl {
            id: commentArea

            width: parent.width
            placeholderText: "Comment"
        }

        LabelControl {
            width: parent.width
            topPadding: 40
            text: "*indicates required field"
        }
    }
}
