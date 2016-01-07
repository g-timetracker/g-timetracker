import QtQuick 2.4
import QtQuick.Layouts 1.3
import Qt.labs.controls 1.0
import TimeLog 1.0

Item {
    id: newDialog

    property bool isModified: checkIsModified()

    property Component toolBar: Component {
        ToolBar {
            RowLayout {
                anchors.fill: parent

                ToolButton {
                    text: newDialog.isModified ? "discard" : "back"
                    label: Image {
                        anchors.centerIn: parent
                        source: newDialog.isModified ? "images/ic_close_white_24dp.png"
                                                     : "images/ic_arrow_back_white_24dp.png"
                    }

                    onClicked: newDialog.close()
                }

                Label {
                    Layout.fillWidth: true
                    text: "New entry"
                }

                ToolButton {
                    enabled: newDialog.isModified && delegateEditor.acceptable
                    text: "Create"
                    onClicked: newDialog.accept()
                }
            }
        }
    }

    property string title: "New entry"

    property alias startTimeAfter: delegateEditor.startTimeAfter
    property alias startTimeBefore: delegateEditor.startTimeBefore
    property int indexBefore

    function checkIsModified() {
        return (!!delegateEditor.category
                || delegateEditor.startTime.valueOf() !== newDialog.startTimeBefore.valueOf() - 1000
                || !!delegateEditor.comment)
    }

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
}
