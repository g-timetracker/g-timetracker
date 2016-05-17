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
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Rectangle {
    id: timeLogDelegate

    property string category
    property date startTime
    property int durationTime
    property string comment
    property date precedingStart
    property date succeedingStart
    property bool isCurrent: false
    property bool isLast: false

    function updateData(category, startTime, comment) {
        if (timeLogDelegate.category !== category) {
            model.category = category
        }
        if (timeLogDelegate.startTime.valueOf() !== startTime.valueOf()) {
            model.startTime = startTime
        }

        if (timeLogDelegate.comment !== comment) {
            model.comment = comment
        }
    }

    implicitHeight: 72
    color: isCurrent ? Material.buttonPressColor : "transparent"


    Column {
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 4

        RowLayout {
            width: parent.width
            spacing: 16

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                font.pixelSize: 16
                elide: Text.ElideLeft
                color: Material.primaryTextColor
                text: timeLogDelegate.category
            }

            Label {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.fillWidth: false
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideRight
                font.pixelSize: 14
                color: Material.secondaryTextColor
                text: TimeTracker.durationText(timeLogDelegate.durationTime, 2,
                                               timeLogDelegate.width < 600) // phone in portrait
            }
        }

        RowLayout {
            width: parent.width
            spacing: 0

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: false
                font.pixelSize: 14
                color: Material.primaryTextColor
                text: TimeTracker.rangeText(timeLogDelegate.startTime,
                                            new Date(timeLogDelegate.succeedingStart.valueOf() - 1000))
            }

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.pixelSize: 14
                color: Material.secondaryTextColor
                text: timeLogDelegate.comment ? " \u2013 " + timeLogDelegate.comment : ""
                visible: !!text
            }
        }
    }

    Rectangle {
        width: parent.width
        height: 1
        y: parent.height - height
        color: Material.dividerColor
        visible: !timeLogDelegate.isLast
    }
}
