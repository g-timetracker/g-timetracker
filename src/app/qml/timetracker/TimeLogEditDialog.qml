import QtQuick 2.4
import QtQuick.Dialogs 1.2

Dialog {
    id: editDialog

    property TimeLogDelegate delegateItem

    function setData() {
        if (!delegateItem) {
            return
        }

        delegateEditor.startTimeAfter = delegateItem.precedingStart
        delegateEditor.startTimeBefore = delegateItem.succeedingStart
        delegateEditor.category = delegateItem.category
        delegateEditor.startTimeCurrent = new Date(delegateItem.startTime)
        delegateEditor.comment = delegateItem.comment
    }

    function openDialog(item) {
        editDialog.delegateItem = item
        setData()
        open()
    }

    signal error(string errorText)

    standardButtons: (delegateEditor.acceptable ? StandardButton.Ok : StandardButton.NoButton)
                     | StandardButton.Cancel | StandardButton.Reset
    title: "Edit entry"

    TimeLogEntryEditor {
        id: delegateEditor

        anchors.left: parent.left
        anchors.right: parent.right
    }

    onAccepted: {
        if (!delegateEditor.acceptable) {
            editDialog.error("Empty category")
            return
        }

        delegateItem.updateData(delegateEditor.category, delegateEditor.startTime,
                                delegateEditor.comment)
        delegateItem = null
    }

    onRejected: delegateItem = null

    onReset: setData()
}
