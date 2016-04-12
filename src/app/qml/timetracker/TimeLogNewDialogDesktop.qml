import QtQuick 2.4
import QtQuick.Dialogs 1.2
import TimeLog 1.0

Dialog {
    id: newDialog

    property alias startTimeAfter: delegateEditor.startTimeAfter
    property alias startTimeBefore: delegateEditor.startTimeBefore
    property int indexBefore

    function setData(indexBefore, timeAfter, timeBefore) {
        newDialog.indexBefore = indexBefore
        newDialog.startTimeAfter = timeAfter
        newDialog.startTimeBefore = timeBefore
    }

    signal dataAccepted(var newData)
    signal error(string errorText)

    standardButtons: (delegateEditor.acceptable ? StandardButton.Ok : StandardButton.NoButton)
                     | StandardButton.Cancel
    title: qsTr("Create new entry")

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
        if (!delegateEditor.acceptable) {
            newDialog.error("Empty category")
            return
        }

        newDialog.dataAccepted(TimeTracker.createTimeLogData(delegateEditor.startTime,
                                                             delegateEditor.category,
                                                             delegateEditor.comment))
    }
}
