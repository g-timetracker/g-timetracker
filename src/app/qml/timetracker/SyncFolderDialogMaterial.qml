import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0
import TimeLog 1.0

Page {
    id: dialog

    property bool isModified: field.text !== (folder.toString().replace(/file:\/\//, ""))

    property string title: qsTranslate("settings", "Sync folder")

    property url folder

    function open() {
        TimeTracker.showDialogRequested(dialog)
    }

    function accept() {
        dialog.close()

        folder = field.text

        dialog.accepted()
    }

    function close() {
        TimeTracker.backRequested()
    }

    signal accepted()

    visible: false

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                text: "back"
                contentItem: Image {
                    fillMode: Image.Pad
                    source: "images/ic_arrow_back_white_24dp.png"
                }

                onClicked: dialog.close()
            }

            LabelControl {
                Layout.fillWidth: true
                Material.theme: Material.Dark
                font.pixelSize: 20
                text: title
            }

            ToolButton {
                enabled: dialog.isModified
                Material.theme: Material.Dark
                font.pixelSize: 14
                text: qsTranslate("dialog", "Select")
                onClicked: dialog.accept()
            }
        }
    }

    onFolderChanged: field.text = (folder.toString().replace(/file:\/\//, ""))

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
                    placeholderText: qsTranslate("settings", "Sync folder")
                    helperText: qsTranslate("settings", "folder, that synchronize between your devices")
                }
            }
        }
    }
}
