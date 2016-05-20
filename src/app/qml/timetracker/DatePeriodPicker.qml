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
    id: datePeriodPicker

    property var beginDate
    property var endDate

    ComboBoxControl {
        id: periodSelector

        property var selectorModel: [
            {
                name: qsTr("day"),
                source: "DateDayPicker.qml"
            },
            {
                name: qsTr("week"),
                source: "DateWeekPicker.qml"
            },
            {
                name: qsTr("month"),
                source: "DateMonthPicker.qml"
            },
            {
                name: qsTr("year"),
                source: "DateYearPicker.qml"
            },
            {
                name: qsTr("all"),
                source: "DateAllPicker.qml"
            },
            {
                name: qsTr("select"),
                source: "DateRangePicker.qml"
            }
        ]

        model: selectorModel
        textRole: "name"
    }

    Loader {
        id: loader

        source: periodSelector.selectorModel[periodSelector.currentIndex]["source"]

        Binding {
            target: datePeriodPicker
            property: "beginDate"
            value: loader.item.beginDate
            when: loader.status === Loader.Ready
        }

        Binding {
            target: datePeriodPicker
            property: "endDate"
            value: loader.item.endDate
            when: loader.status === Loader.Ready
        }
    }
}
