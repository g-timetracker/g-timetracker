import QtQuick 2.4
import Qt.labs.controls 1.0
import TimeLog 1.0

Item {
    id: editDialog

    property string title: "Edit"

    property TimeLogDelegate delegateItem

    function setData(item) {
        delegateItem = item

        if (!delegateItem) {
            return
        }

        delegateEditor.startTimeAfter = delegateItem.precedingStart
        delegateEditor.startTimeBefore = delegateItem.succeedingStart
        delegateEditor.category = delegateItem.category
        delegateEditor.startTimeCurrent = new Date(delegateItem.startTime)
        delegateEditor.comment = delegateItem.comment
    }

    function accept() {
        editDialog.close()

        if (!delegateEditor.acceptable) {
            editDialog.error("Empty category")
            return
        }

        delegateItem.updateData(delegateEditor.category, delegateEditor.startTime,
                                delegateEditor.comment)
        delegateItem = null
    }

    function reject() {
        editDialog.close()

        delegateItem = null
    }

    function close() {
        MainWindow.back()
    }

    signal error(string errorText)

    visible: false

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 10

        TimeLogEntryEditor {
            id: delegateEditor

            anchors.left: parent.left
            anchors.right: parent.right
        }

        Row {
            anchors.right: parent.right
            spacing: 10

            Button {
                text: "Discard"
                onClicked: editDialog.reject()
            }

            Button {
                text: "Save"
                onClicked: editDialog.accept()
            }
        }
    }
}
