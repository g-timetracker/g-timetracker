import QtQuick 2.4
import QtQuick.Controls 2.0

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
            placeholderText: qsTr("Name*")
            helperText: qsTr("to split the name into subcategories, use \u2018>\u2019")
        }

        TextAreaControl {
            id: commentArea

            width: parent.width
            placeholderText: qsTr("Comment")
        }

        LabelControl {
            width: parent.width
            topPadding: 40
            text: qsTr("*indicates required field")
        }
    }
}
