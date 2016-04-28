import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: dialog

    property bool isModified: checkIsModified()

    property var delegateItem   // TODO: change to TimeLogDelegate when fixed in Qt

    function checkIsModified() {
        return !(delegateItem
                 && delegateEditor.category === delegateItem.category
                 && delegateEditor.startTime.valueOf() === delegateItem.startTime.valueOf()
                 && delegateEditor.comment === delegateItem.comment)
    }

    function setData(item) {
        delegateItem = item

        if (!delegateItem) {
            return
        }

        delegateEditor.startTimeAfter = delegateItem.precedingStart
        delegateEditor.startTimeBefore = delegateItem.succeedingStart
        delegateEditor.category = delegateItem.category
        delegateEditor.startTimeCurrent = new Date(delegateItem.startTime)
        delegateEditor.comment = delegateItem.comment
    }

    function accept() {
        dialog.close()

        if (!delegateEditor.acceptable) {
            dialog.error("Empty category")
            return
        }

        delegateItem.updateData(delegateEditor.category, delegateEditor.startTime,
                                delegateEditor.comment)
        delegateItem = null
    }

    function reject() {
        dialog.close()

        delegateItem = null
    }

    function close() {
        TimeTracker.backRequested()
    }

    signal error(string errorText)

    title: qsTr("Edit entry")
    visible: false

    header: ToolBarMaterial {
        title: dialog.title
        leftIcon: dialog.isModified ? "images/ic_close_white_24dp.png"
                                    : "images/ic_arrow_back_white_24dp.png"
        rightText: qsTr("Save")
        rightEnabled: dialog.isModified && delegateEditor.acceptable

        onLeftActivated: {
            if (dialog.isModified && Settings.isConfirmationsEnabled) {
                discardConfirmationDialog.open()
            } else {
                dialog.reject()
            }
        }
        onRightActivated: dialog.accept()
    }

    MessageDialogMaterial {
        id: discardConfirmationDialog

        text: qsTr("Discard entry changes?")
        affirmativeText: qsTranslate("dialog", "Discard")

        onAccepted: dialog.reject()
    }

    Flickable {
        anchors.fill: parent
        anchors.bottomMargin: Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio
        contentWidth: delegateEditor.width
        contentHeight: delegateEditor.height
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { }

        TimeLogEntryEditor {
            id: delegateEditor

            width: dialog.width
        }
    }
}
