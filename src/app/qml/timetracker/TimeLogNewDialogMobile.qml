import QtQuick 2.4
import Qt.labs.controls 1.0
import TimeLog 1.0

Item {
    id: newDialog

    property string title: "New entry"

    property alias startTimeAfter: delegateEditor.startTimeAfter
    property alias startTimeBefore: delegateEditor.startTimeBefore
    property int indexBefore

    function setData(indexBefore, timeAfter, timeBefore) {
        newDialog.indexBefore = indexBefore
        newDialog.startTimeAfter = timeAfter
        newDialog.startTimeBefore = timeBefore
    }

    function accept() {
        newDialog.close()

        if (!delegateEditor.acceptable) {
            newDialog.error("Empty category")
            return
        }

        newDialog.dataAccepted(TimeTracker.createTimeLogData(delegateEditor.startTime,
                                                             delegateEditor.category,
                                                             delegateEditor.comment))
    }

    function close() {
        MainWindow.back()
    }

    signal dataAccepted(var newData)
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
                onClicked: newDialog.close()
            }

            Button {
                text: "Create"
                onClicked: newDialog.accept()
            }
        }
    }

    onVisibleChanged: {
        if (!visible) {
            return
        }

        delegateEditor.category = ""
        delegateEditor.startTimeCurrent = new Date(startTimeBefore.valueOf() - 1000)
        delegateEditor.comment = ""
    }
}
