import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: dialog

    property bool isModified: checkIsModified()

    property string title: qsTr("Edit category")

    property CategoryDelegate delegateItem

    function checkIsModified() {
        return !(delegateItem
                 && editor.categoryName === delegateItem.fullName
                 && (!editor.categoryComment ? !delegateItem.categoryData || !delegateItem.categoryData["comment"]
                                             : delegateItem.categoryData && delegateItem.categoryData["comment"]
                                               && editor.categoryComment === delegateItem.categoryData["comment"]))
    }

    function setData(item) {
        delegateItem = item

        if (!delegateItem) {
            return
        }

        editor.categoryName = delegateItem.fullName
        editor.categoryComment = delegateItem.categoryData["comment"] ? delegateItem.categoryData["comment"] : ""
    }

    function accept() {
        dialog.close()

        var data = delegateItem.categoryData
        if (!!editor.categoryComment) {
            data["comment"] = editor.categoryComment
        } else {
            delete data["comment"]
        }

        delegateItem.updateData(TimeTracker.createTimeLogCategoryData(editor.categoryName, data))
        delegateItem = null
    }

    function reject() {
        dialog.close()

        delegateItem = null
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
                contentItem: Image {
                    fillMode: Image.Pad
                    source: dialog.isModified ? "images/ic_close_white_24dp.png"
                                              : "images/ic_arrow_back_white_24dp.png"
                }

                onClicked: {
                    if (dialog.isModified && Settings.isConfirmationsEnabled) {
                        discardConfirmationDialog.open()
                    } else {
                        dialog.reject()
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
                enabled: dialog.isModified && editor.acceptable
                Material.theme: Material.Dark
                font.pixelSize: 14
                text: qsTr("Save")
                onClicked: dialog.accept()
            }
        }
    }

    MessageDialogMaterial {
        id: discardConfirmationDialog

        text: qsTr("Discard category changes?")
        affirmativeText: qsTranslate("dialog", "Discard")

        onAccepted: dialog.reject()
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
