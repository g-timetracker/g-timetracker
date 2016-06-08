/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

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

    onAccepted: AppSettings.syncPath = folder

    CreateFolderDialogMaterial {
        id: createDialog

        path: folderModel.path
    }

    FolderListModel {
        id: folderModel

        property string path: TimeTracker.urlToLocalFile(folder)

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
