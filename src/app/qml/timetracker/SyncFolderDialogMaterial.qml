import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import Qt.labs.folderlistmodel 2.1
import TimeLog 1.0

Page {
    id: dialog

    property string title: qsTranslate("settings", "Sync folder")

    property alias folder: folderModel.folder

    function open() {
        TimeTracker.showDialogRequested(dialog)
    }

    function accept() {
        dialog.close()

        dialog.accepted()
    }

    function close() {
        TimeTracker.backRequested()
    }

    signal accepted()

    visible: false

    header: ToolBar {
        height: 48 * 2
        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true

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
                    Material.theme: Material.Dark
                    font.pixelSize: 14
                    text: qsTranslate("dialog", "Select")
                    onClicked: dialog.accept()
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Material.theme: Material.Dark
                leftPadding: 52 // TODO: 72
                rightPadding: 16
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 20
                elide: Text.ElideLeft
                text: dialog.folder.toString().replace(/file:\/\//, "")
            }
        }
    }

    FolderListModel {
        id: folderModel

        showFiles: false
        showDotAndDotDot: true
        showOnlyReadable: true
    }

    ListView {
        anchors.fill: parent

        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { }

        model: folderModel
        delegate: ItemDelegate {
            width: parent.width
            // Don't show current dir and parent dir for /
            visible: text !== "." && (text !== ".." || folderModel.folder.toString() !== "file:///")
            height: visible ? implicitHeight : 0
            text: model.fileName
            onClicked: folderModel.folder = (model.fileName === ".." ? folderModel.parentFolder
                                                                     : model.fileURL)
        }
    }
}
