import QtQuick 2.4
import QtQuick.Dialogs 1.2

Dialog {
    id: editDialog

    property alias startTimeAfter: delegateEditor.startTimeAfter
    property alias startTimeBefore: delegateEditor.startTimeBefore
    property TimeLogDelegate delegateItem

    function setData() {
        if (!delegateItem) {
            return
        }

        delegateEditor.category = delegateItem.category
        delegateEditor.startTimeCurrent = new Date(delegateItem.startTime)
        delegateEditor.comment = delegateItem.comment
    }

    function openDialog(item, timeAfter, timeBefore) {
        editDialog.startTimeAfter = timeAfter
        editDialog.startTimeBefore = timeBefore
        editDialog.delegateItem = item
        setData()
        open()
    }

    width: delegateEditor.implicitWidth
    standardButtons: StandardButton.Ok | StandardButton.Cancel | StandardButton.Reset
    title: "Edit entry"

    TimeLogEntryEditor {
        id: delegateEditor
    }

    onAccepted: {
        delegateItem.updateData(delegateEditor.category, delegateEditor.startTime,
                                delegateEditor.comment)
        delegateItem = null
    }

    onRejected: delegateItem = null

    onReset: setData()
}
