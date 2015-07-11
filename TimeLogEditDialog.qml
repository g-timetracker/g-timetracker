import QtQuick 2.4
import QtQuick.Dialogs 1.2

Dialog {
    id: editDialog

    property TimeLogDelegate delegateItem

    function setData() {
        if (!delegateItem) {
            return
        }

        delegateEditor.category = delegateItem.category
        delegateEditor.startTime = new Date(delegateItem.startTime)
        delegateEditor.durationTime = 0
        delegateEditor.comment = delegateItem.comment
    }

    width: delegateEditor.implicitWidth
    standardButtons: StandardButton.Ok | StandardButton.Cancel | StandardButton.Reset
    title: "Edit entry"

    TimeLogEntryEditor {
        id: delegateEditor
    }

    onDelegateItemChanged: setData()

    onAccepted: {
        delegateItem.updateData(delegateEditor.category, delegateEditor.startTime,
                                delegateEditor.comment)
        delegateItem = null
    }

    onRejected: delegateItem = null

    onReset: setData()
}
