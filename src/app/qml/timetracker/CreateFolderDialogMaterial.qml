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
