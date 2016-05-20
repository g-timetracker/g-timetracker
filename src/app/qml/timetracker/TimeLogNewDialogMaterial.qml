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

    property alias startTimeAfter: delegateEditor.startTimeAfter
    property alias startTimeBefore: delegateEditor.startTimeBefore
    property int indexBefore

    function checkIsModified() {
        return (!!delegateEditor.category
                || delegateEditor.startTime.valueOf() !== dialog.startTimeBefore.valueOf() - 1000
                || !!delegateEditor.comment)
    }

    function setData(indexBefore, timeAfter, timeBefore) {
        dialog.indexBefore = indexBefore
        dialog.startTimeAfter = timeAfter
        dialog.startTimeBefore = timeBefore
    }

    function accept() {
        dialog.close()

        if (!delegateEditor.acceptable) {
            dialog.error("Empty category")
            return
        }

        dialog.dataAccepted(TimeTracker.createTimeLogData(delegateEditor.startTime,
                                                          delegateEditor.category,
                                                          delegateEditor.comment))
    }

    function close() {
        TimeTracker.backRequested()
    }

    signal dataAccepted(var newData)
    signal error(string errorText)

    title: qsTr("New entry")
    visible: false

    header: ToolBarMaterial {
        title: dialog.title
        leftIcon: dialog.isModified ? "images/ic_close_white_24dp.png"
                                    : "images/ic_arrow_back_white_24dp.png"
        rightText: qsTr("Create")
        rightEnabled: dialog.isModified && delegateEditor.acceptable

        onLeftActivated: {
            if (dialog.isModified && AppSettings.isConfirmationsEnabled) {
                discardConfirmationDialog.open()
            } else {
                dialog.close()
            }
        }
        onRightActivated: dialog.accept()
    }

    MessageDialogMaterial {
        id: discardConfirmationDialog

        text: qsTr("Discard new entry?")
        affirmativeText: qsTranslate("dialog", "Discard")

        onAccepted: dialog.close()
    }

    Flickable {
        anchors.bottomMargin: Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio
        anchors.fill: parent
        contentWidth: delegateEditor.width
        contentHeight: delegateEditor.height
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { }

        TimeLogEntryEditor {
            id: delegateEditor

            width: dialog.width
        }
    }

    onVisibleChanged: {
        if (!visible) {
            return
        }

        delegateEditor.category = ""
        delegateEditor.startTimeCurrent = new Date(startTimeBefore.valueOf() - 1000)
        delegateEditor.comment = ""
    }
}
