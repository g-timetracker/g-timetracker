import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: dialog

    property bool isModified: checkIsModified()

    function checkIsModified() {
        return (!!editor.categoryName || !!editor.categoryComment)
    }

    function setData(name) {
        editor.categoryName = name ? name : ""
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

    title: qsTr("New category")
    visible: false

    header: ToolBarMaterial {
        title: dialog.title
        leftIcon: dialog.isModified ? "images/ic_close_white_24dp.png"
                                    : "images/ic_arrow_back_white_24dp.png"
        rightText: qsTr("Create")
        rightEnabled: dialog.isModified && editor.acceptable

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

        text: qsTr("Discard new category?")
        affirmativeText: qsTranslate("dialog", "Discard")

        onAccepted: dialog.close()
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
