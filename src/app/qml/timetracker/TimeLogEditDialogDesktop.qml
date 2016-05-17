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
import QtQuick.Dialogs 1.2

Dialog {
    id: editDialog

    property TimeLogDelegate delegateItem

    function setData(item) {
        delegateItem = item

        if (!delegateItem) {
            return
        }

        delegateEditor.startTimeAfter = delegateItem.precedingStart
        delegateEditor.startTimeBefore = delegateItem.succeedingStart
        delegateEditor.category = delegateItem.category
        delegateEditor.startTimeCurrent = new Date(delegateItem.startTime)
        delegateEditor.comment = delegateItem.comment
    }

    signal error(string errorText)

    standardButtons: (delegateEditor.acceptable ? StandardButton.Ok : StandardButton.NoButton)
                     | StandardButton.Cancel | StandardButton.Reset
    title: qsTr("Edit entry")

    TimeLogEntryEditor {
        id: delegateEditor

        anchors.left: parent.left
        anchors.right: parent.right
    }

    onAccepted: {
        if (!delegateEditor.acceptable) {
            editDialog.error("Empty category")
            return
        }

        delegateItem.updateData(delegateEditor.category, delegateEditor.startTime,
                                delegateEditor.comment)
        delegateItem = null
    }

    onRejected: delegateItem = null

    onReset: setData(delegateItem)
}
