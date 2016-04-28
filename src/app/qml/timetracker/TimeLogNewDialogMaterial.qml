import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: dialog

    property bool isModified: checkIsModified()

    property alias startTimeAfter: delegateEditor.startTimeAfter
    property alias startTimeBefore: delegateEditor.startTimeBefore
    property int indexBefore

    function checkIsModified() {
        return (!!delegateEditor.category
                || delegateEditor.startTime.valueOf() !== dialog.startTimeBefore.valueOf() - 1000
                || !!delegateEditor.comment)
    }

    function setData(indexBefore, timeAfter, timeBefore) {
        dialog.indexBefore = indexBefore
        dialog.startTimeAfter = timeAfter
        dialog.startTimeBefore = timeBefore
    }

    function accept() {
        dialog.close()

        if (!delegateEditor.acceptable) {
            dialog.error("Empty category")
            return
        }

        dialog.dataAccepted(TimeTracker.createTimeLogData(delegateEditor.startTime,
                                                          delegateEditor.category,
                                                          delegateEditor.comment))
    }

    function close() {
        TimeTracker.backRequested()
    }

    signal dataAccepted(var newData)
    signal error(string errorText)

    title: qsTr("New entry")
    visible: false

    header: ToolBarMaterial {
        title: dialog.title
        leftIcon: dialog.isModified ? "images/ic_close_white_24dp.png"
                                    : "images/ic_arrow_back_white_24dp.png"
        rightText: qsTr("Create")
        rightEnabled: dialog.isModified && delegateEditor.acceptable

        onLeftActivated: {
            if (dialog.isModified && Settings.isConfirmationsEnabled) {
                discardConfirmationDialog.open()
            } else {
                dialog.close()
            }
        }
        onRightActivated: dialog.accept()
    }

    MessageDialogMaterial {
        id: discardConfirmationDialog

        text: qsTr("Discard new entry?")
        affirmativeText: qsTranslate("dialog", "Discard")

        onAccepted: dialog.close()
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

            width: dialog.width
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
