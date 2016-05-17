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
import QtQuick.Controls 2.0
import TimeLog 1.0
import "Util.js" as Util
import "Texts.js" as Texts

Item {
    id: delegateEditor

    property date startTimeAfter: new Date(0)
    property date startTimeBefore: new Date()
    property var startTimeCurrent: new Date()

    property alias category: categoryPicker.category
    property var startTime: new Date()
    property alias comment: commentArea.text

    property bool acceptable: !!category

    property bool singleColumn: false

    implicitHeight: container.height + container.anchors.margins * 2

    onStartTimeCurrentChanged: {
        if (timeEditor.startTimeCurrent != startTimeCurrent) {
            timeEditor.startTimeCurrent = startTimeCurrent
        }
    }

    QtObject {
        id: d

        property int durationTime: Util.calcDuration(startTime, startTimeBefore)
    }

    Column {
        id: container

        anchors.margins: 16
        anchors.topMargin: 24
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 16

        CategoryPicker {
            id: categoryPicker
        }

        LabelControl {
            text: Texts.labelText(qsTranslate("entry editor", "Start date"))
        }

        DatePicker {
            id: calendar

            property alias origDate: delegateEditor.startTimeCurrent

            minimumDate: new Date(startTimeAfter.valueOf() + 1000)
            maximumDate: new Date(startTimeBefore.valueOf() - 1000)

            onOrigDateChanged: selectedDate = origDate
        }

        LabelControl {
            text: Texts.labelText(qsTranslate("entry editor", "Start time"))
        }

        TimeEditor {
            id: timeEditor

            startDateCurrent: calendar.selectedDate
            minDateTime: new Date(startTimeAfter.valueOf() + 1000)
            maxDateTime: new Date(startTimeBefore.valueOf() - 1000)

            onStartTimeCurrentChanged: {
                if (delegateEditor.startTime != startTimeCurrent) {
                    delegateEditor.startTime = startTimeCurrent
                }
            }
        }

        LabelControl {
            text: Texts.labelText(qsTranslate("entry editor", "Duration"))
        }

        Label {
            elide: Text.ElideRight
            text: TimeTracker.durationText(d.durationTime)
        }

        TextAreaControl {
            id: commentArea

            property alias origComment: delegateEditor.comment

            width: parent.width
            placeholderText: Texts.labelText(qsTranslate("entry editor", "Comment"))
        }
    }
}
