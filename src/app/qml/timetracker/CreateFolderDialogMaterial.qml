import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: dialog

    property bool isModified: !!field.text

    property string path

    function open() {
        field.text = ""
        TimeTracker.showDialogRequested(dialog)
    }

    function accept() {
        if (!TimeTracker.createFolder(dialog.path, field.text)) {
            TimeTracker.error(qsTranslate("dialog", "Fail to create directory"));
        } else {
            dialog.close()
        }
    }

    function close() {
        TimeTracker.backRequested()
    }

    title: qsTranslate("dialog", "Create directory")
    visible: false

    header: ToolBarMaterial {
        title: dialog.title
        leftIcon: "images/ic_arrow_back_white_24dp.png"
        rightText: qsTranslate("dialog", "Create")
        rightEnabled: dialog.isModified

        onLeftActivated: dialog.close()
        onRightActivated: dialog.accept()
    }

    Flickable {
        anchors.bottomMargin: Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio
        anchors.fill: parent
        contentWidth: settingsItem.width
        contentHeight: settingsItem.height
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { }

        Item {
            id: settingsItem

            width: dialog.width
            implicitHeight: container.height + container.anchors.margins * 2

            Column {
                id: container

                anchors.margins: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 16

                TextFieldControl {
                    id: field

                    width: parent.width
                    inputMethodHints: Qt.ImhNoPredictiveText
                    placeholderText: qsTranslate("dialog", "Directory name")
                    helperText: dialog.path + (/\/$/.test(dialog.path) ? "" : "/") + text
                }
            }
        }
    }
}
