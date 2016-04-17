import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0
import TimeLog 1.0

Page {
    id: newDialog

    property bool isModified: checkIsModified()

    property string title: qsTr("New entry")

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
        TimeTracker.backRequested()
    }

    signal dataAccepted(var newData)
    signal error(string errorText)

    visible: false

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                text: newDialog.isModified ? "discard" : "back"
                contentItem: Image {
                    fillMode: Image.Pad
                    source: newDialog.isModified ? "images/ic_close_white_24dp.png"
                                                 : "images/ic_arrow_back_white_24dp.png"
                }

                onClicked: newDialog.close()
            }

            LabelControl {
                Layout.fillWidth: true
                Material.theme: Material.Dark
                font.pixelSize: 20
                text: title
            }

            ToolButton {
                enabled: newDialog.isModified && delegateEditor.acceptable
                Material.theme: Material.Dark
                font.pixelSize: 14
                text: qsTr("Create")
                onClicked: newDialog.accept()
            }
        }
    }

    Flickable {
        anchors.bottomMargin: Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio
        anchors.fill: parent
        contentWidth: delegateEditor.width
        contentHeight: delegateEditor.height
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { }

        TimeLogEntryEditor {
            id: delegateEditor

            width: newDialog.width
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
