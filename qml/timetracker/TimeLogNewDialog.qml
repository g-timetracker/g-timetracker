import QtQuick 2.4
import QtQuick.Dialogs 1.2

Dialog {
    id: newDialog

    property alias startTimeAfter: delegateEditor.startTimeAfter
    property alias startTimeBefore: delegateEditor.startTimeBefore
    property int indexBefore

    function openDialog(indexBefore, timeAfter, timeBefore) {
        newDialog.indexBefore = indexBefore
        newDialog.startTimeAfter = timeAfter
        newDialog.startTimeBefore = timeBefore
        open()
    }

    signal dataAccepted(var newData)

    standardButtons: StandardButton.Ok | StandardButton.Cancel
    title: "Add new entry"

    TimeLogEntryEditor {
        id: delegateEditor

        anchors.left: parent.left
        anchors.right: parent.right
    }

    onVisibleChanged: {
        if (!visible) {
            return
        }

        delegateEditor.category = ""
        delegateEditor.startTimeCurrent = new Date(startTimeBefore.valueOf() - 1000)
        delegateEditor.comment = ""
    }

    onAccepted: {
        newDialog.dataAccepted(TimeLog.createTimeLogData(delegateEditor.startTime, 0,
                                                         delegateEditor.category,
                                                         delegateEditor.comment))
    }
}
