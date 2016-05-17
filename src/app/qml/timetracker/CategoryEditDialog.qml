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

    property bool isModified: checkIsModified()

    property var delegateItem   // TODO: change to CategoryDelegate when fixed in Qt

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

    title: qsTr("Edit category")
    visible: false

    header: ToolBarMaterial {
        title: dialog.title
        leftIcon: dialog.isModified ? "images/ic_close_white_24dp.png"
                                    : "images/ic_arrow_back_white_24dp.png"
        rightText: qsTr("Save")
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
