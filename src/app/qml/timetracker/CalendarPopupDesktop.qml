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

import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.4
import TimeLog 1.0

Window {
    id: popup

    property alias minimumDate: calendar.minimumDate
    property alias maximumDate: calendar.maximumDate
    property alias selectedDate: calendar.selectedDate

    function showPopup(parentControl) {
        d.positionPopup(parentControl)
        popup.show()
        popup.requestActivate()
    }

    x: d.position.x
    y: d.position.y
    height: calendar.implicitHeight
    width: calendar.implicitWidth
    flags: Qt.Popup

    onFocusObjectChanged: {
        if (!object) {
            hide()
        }
    }

    QtObject {
        id: d

        property point position

        function positionPopup(parentControl) {
            var parentControlPosition = parentControl.mapToGlobal(0, 0)
            var popupX = parentControlPosition.x
            popupX -= popupX + popup.width > Screen.desktopAvailableWidth ? popup.width - parentControl.width : 0
            var popupY = parentControlPosition.y
            popupY -= popupY - popup.height < 0 ? - parentControl.height : popup.height
            position = Qt.point(popupX, popupY)
        }
    }

    Calendar {
        id: calendar

        anchors.fill: parent

        onClicked: popup.hide()
    }
}
