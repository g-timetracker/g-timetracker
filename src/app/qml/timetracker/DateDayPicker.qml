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

ItemPositioner {
    id: picker

    property var beginDate: pickerLoader.selectedDate
    property var endDate: new Date(pickerLoader.selectedDate.valueOf() + 86399000)

    ComboBoxControl {
        id: periodSelector


        model: [
            qsTr("current"),
            qsTr("previous"),
            qsTr("select")
        ]

        onCurrentIndexChanged: {
            switch (currentIndex) {
            case 0:
                pickerLoader.selectedDate = new Date(new Date().setHours(0, 0, 0, 0))
                break
            case 1:
                pickerLoader.selectedDate = new Date(new Date(Date.now() - 86400000).setHours(0, 0, 0, 0))
                break
            }
        }
    }

    Loader {
        id: pickerLoader

        property date selectedDate

        source: "DatePicker.qml"
        active: periodSelector.currentIndex === 2
        visible: active

        Binding {
            target: pickerLoader
            property: "selectedDate"
            value: pickerLoader.item ? pickerLoader.item.selectedDate : new Date()
            when: pickerLoader.active && pickerLoader.status === Loader.Ready
        }

        Binding {
            target: pickerLoader.item
            property: "selectedDate"
            value: pickerLoader.selectedDate
            when: pickerLoader.active && loader.status === Loader.Ready
        }
    }
}
