import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0
import TimeLog 1.0

Page {
    id: dialog

    property bool isModified: checkIsModified()

    property string title: "New category"

    function checkIsModified() {
        return (!!editor.categoryName || !!editor.categoryComment)
    }

    function setData() {
        editor.categoryName = ""
        editor.categoryComment = ""
    }

    function accept() {
        dialog.close()

        var data = {}
        if (!!editor.categoryComment) {
            data["comment"] = editor.categoryComment
        }

        dialog.dataAccepted(TimeTracker.createTimeLogCategoryData(editor.categoryName, data))
    }

    function reject() {
        dialog.close()
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
                text: dialog.isModified ? "discard" : "back"
                label: Image {
                    anchors.centerIn: parent
                    source: dialog.isModified ? "images/ic_close_white_24dp.png"
                                              : "images/ic_arrow_back_white_24dp.png"
                }

                onClicked: dialog.reject()
            }

            LabelControl {
                Layout.fillWidth: true
                Material.theme: Material.Dark
                font.pixelSize: 20
                text: title
            }

            ToolButton {
                enabled: dialog.isModified && editor.acceptable
                Material.theme: Material.Dark
                font.pixelSize: 14
                text: "Create"
                onClicked: dialog.accept()
            }
        }
    }

    Flickable {
        anchors.fill: parent
        anchors.bottomMargin: Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio
        contentWidth: editor.width
        contentHeight: editor.height
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { }

        CategoryEditor {
            id: editor

            width: dialog.width
        }
    }
}
