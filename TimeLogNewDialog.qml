import QtQuick 2.4
import QtQuick.Dialogs 1.2

Dialog {
    id: newDialog

    property int beforeIndex

    function openDialog(beforeIndex) {
        newDialog.beforeIndex = (beforeIndex === undefined) ? -1 : beforeIndex
        open()
    }

    signal dataAccepted(var newData)

    width: delegateEditor.implicitWidth
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    title: "Add new entry"

    TimeLogEntryEditor {
        id: delegateEditor
    }

    onVisibleChanged: {
        if (!visible) {
            return
        }

        delegateEditor.category = ""
        delegateEditor.startTime = new Date()
        delegateEditor.durationTime = 0
        delegateEditor.comment = ""
    }

    onAccepted: {
        newDialog.dataAccepted(TimeLog.createTimeLogData(delegateEditor.startTime, 0,
                                                         delegateEditor.category,
                                                         delegateEditor.comment))
    }
}
