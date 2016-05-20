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
import QtQuick.Controls 1.4 as QQC1 // FIXME: crashes without this
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

TextField {
    id: datePicker

    property alias minimumDate: popup.minimumDate
    property alias maximumDate: popup.maximumDate
    property alias selectedDate: popup.selectedDate

    implicitWidth: PlatformMaterial.dropdownWidth
    rightPadding: icon.width + 16
    readOnly: true
    text: Qt.formatDate(selectedDate, "ddd, MMM d yyyy")

    MouseArea {
        anchors.fill: parent

        onClicked: popup.showPopup(datePicker)
    }

    IconMaterial {
        id: icon

        anchors.topMargin: datePicker.topPadding
        anchors.bottomMargin: datePicker.bottomPadding
        anchors.top: datePicker.top
        anchors.bottom: datePicker.bottom
        anchors.right: datePicker.right
        source: "images/ic_arrow_drop_down_white_24dp.png"
    }

    CalendarPopup {
        id: popup
    }
}
