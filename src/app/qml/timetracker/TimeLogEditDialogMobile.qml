import QtQuick 2.4
import QtQuick.Layouts 1.3
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0
import TimeLog 1.0

Item {
    id: editDialog

    property bool isModified: checkIsModified()

    property Component toolBar: Component {
        ToolBar {
            RowLayout {
                anchors.fill: parent

                ToolButton {
                    text: editDialog.isModified ? "discard" : "back"
                    label: Image {
                        anchors.centerIn: parent
                        source: editDialog.isModified ? "images/ic_close_white_24dp.png"
                                                      : "images/ic_arrow_back_white_24dp.png"
                    }

                    onClicked: editDialog.reject()
                }

                Label {
                    Layout.fillWidth: true
                    Material.theme: Material.Dark
                    font.pixelSize: 20
                    text: "Edit"
                }

                ToolButton {
                    enabled: editDialog.isModified && delegateEditor.acceptable
                    Material.theme: Material.Dark
                    font.pixelSize: 14
                    text: "Save"
                    onClicked: editDialog.accept()
                }
            }
        }
    }

    property string title: "Edit"

    property TimeLogDelegate delegateItem

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
        MainWindow.back()
    }

    signal error(string errorText)

    visible: false

    TimeLogEntryEditor {
        id: delegateEditor

        anchors.left: parent.left
        anchors.right: parent.right
    }
}
