import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0
import TimeLog 1.0

Page {
    id: dialog

    property bool isModified: checkIsModified()

    property string title: qsTr("Edit category")

    property string categoryName
    property var categoryData

    function checkIsModified() {
        return editor.categoryName !== categoryName
                || (!editor.categoryComment ? !!categoryData && !!categoryData["comment"]
                                            : !categoryData || !categoryData["comment"]
                                              || editor.categoryComment !== categoryData["comment"])
    }

    function setData(name, data) {
        editor.categoryName = categoryName = name
        categoryData = data ? data : {}
        editor.categoryComment = categoryData["comment"] ? categoryData["comment"] : ""
    }

    function accept() {
        dialog.close()

        var data = categoryData
        if (!!editor.categoryComment) {
            data["comment"] = editor.categoryComment
        } else {
            delete data["comment"]
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
                text: qsTr("Save")
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
