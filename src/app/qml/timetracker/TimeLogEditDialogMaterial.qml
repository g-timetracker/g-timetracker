import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: editDialog

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
        editDialog.close()

        if (!delegateEditor.acceptable) {
            editDialog.error("Empty category")
            return
        }

        delegateItem.updateData(delegateEditor.category, delegateEditor.startTime,
                                delegateEditor.comment)
        delegateItem = null
    }

    function reject() {
        editDialog.close()

        delegateItem = null
    }

    function close() {
        TimeTracker.backRequested()
    }

    signal error(string errorText)

    title: qsTr("Edit entry")
    visible: false

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                text: editDialog.isModified ? "discard" : "back"
                contentItem: Image {
                    fillMode: Image.Pad
                    source: editDialog.isModified ? "images/ic_close_white_24dp.png"
                                                  : "images/ic_arrow_back_white_24dp.png"
                }

                onClicked: {
                    if (editDialog.isModified && Settings.isConfirmationsEnabled) {
                        discardConfirmationDialog.open()
                    } else {
                        editDialog.reject()
                    }
                }
            }

            LabelControl {
                Layout.fillWidth: true
                Material.theme: Material.Dark
                font.pixelSize: 20
                text: title
            }

            ToolButton {
                enabled: editDialog.isModified && delegateEditor.acceptable
                Material.theme: Material.Dark
                font.pixelSize: 14
                text: qsTr("Save")
                onClicked: editDialog.accept()
            }
        }
    }

    MessageDialogMaterial {
        id: discardConfirmationDialog

        text: qsTr("Discard entry changes?")
        affirmativeText: qsTranslate("dialog", "Discard")

        onAccepted: editDialog.reject()
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

            width: editDialog.width
        }
    }
}
