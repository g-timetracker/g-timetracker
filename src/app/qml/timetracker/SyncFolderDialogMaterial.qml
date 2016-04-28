import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import Qt.labs.folderlistmodel 2.1
import TimeLog 1.0

Page {
    id: dialog

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

    title: qsTranslate("settings", "Sync folder")
    visible: false

    header: ToolBarMaterial {
        height: 48 * 2
        title: dialog.title
        rightText: qsTranslate("dialog", "Select")

        onLeftActivated: dialog.close()
        onRightActivated: dialog.accept()

            Label {
            width: parent.width
            height: 48
            y: parent.height - height
            leftPadding: 72
            rightPadding: 16
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 20
            elide: Text.ElideLeft
            text: folderModel.path
        }
    }

    onAccepted: Settings.syncPath = folder

    CreateFolderDialogMaterial {
        id: createDialog

        path: folderModel.path
    }

    FolderListModel {
        id: folderModel

        property string path: folder.toString().replace(/file:\/\//, "")

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
            visible: text !== "." && (text !== ".." || folderModel.path !== "/")
            height: visible ? implicitHeight : 0
            text: model.fileName
            onClicked: folderModel.folder = (model.fileName === ".." ? folderModel.parentFolder
                                                                     : model.fileURL)
        }
    }

    FloatingActionButton {
        iconSource: "images/ic_create_new_folder_white_24dp.png"

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: height / 2
        onClicked: createDialog.open()
    }
}
