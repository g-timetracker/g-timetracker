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
import TimeLog 1.0

Dialog {
    id: newDialog

    property alias startTimeAfter: delegateEditor.startTimeAfter
    property alias startTimeBefore: delegateEditor.startTimeBefore
    property int indexBefore

    function setData(indexBefore, timeAfter, timeBefore) {
        newDialog.indexBefore = indexBefore
        newDialog.startTimeAfter = timeAfter
        newDialog.startTimeBefore = timeBefore
    }

    signal dataAccepted(var newData)
    signal error(string errorText)

    standardButtons: (delegateEditor.acceptable ? StandardButton.Ok : StandardButton.NoButton)
                     | StandardButton.Cancel
    title: qsTr("Create new entry")

    TimeLogEntryEditor {
        id: delegateEditor

        anchors.left: parent.left
        anchors.right: parent.right
    }

    onVisibleChanged: {
        if (!visible) {
            return
        }

        delegateEditor.category = ""
        delegateEditor.startTimeCurrent = new Date(startTimeBefore.valueOf() - 1000)
        delegateEditor.comment = ""
    }

    onAccepted: {
        if (!delegateEditor.acceptable) {
            newDialog.error("Empty category")
            return
        }

        newDialog.dataAccepted(TimeTracker.createTimeLogData(delegateEditor.startTime,
                                                             delegateEditor.category,
                                                             delegateEditor.comment))
    }
}
