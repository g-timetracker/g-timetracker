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

        model: [
            qsTr("day"),
            qsTr("month"),
            qsTr("year"),
            qsTr("all"),
            qsTr("select")
        ]
    }

    DateDayPicker {
        id: dateDayPicker

        property bool isCurrent: periodSelector.currentIndex == 0

        visible: isCurrent

        Binding {
            target: datePeriodPicker
            property: "beginDate"
            value: dateDayPicker.beginDate
            when: dateDayPicker.isCurrent
        }

        Binding {
            target: datePeriodPicker
            property: "endDate"
            value: dateDayPicker.endDate
            when: dateDayPicker.isCurrent
        }
    }

    DateMonthPicker {
        id: dateMonthPicker

        property bool isCurrent: periodSelector.currentIndex == 1

        visible: isCurrent

        Binding {
            target: datePeriodPicker
            property: "beginDate"
            value: dateMonthPicker.beginDate
            when: dateMonthPicker.isCurrent
        }

        Binding {
            target: datePeriodPicker
            property: "endDate"
            value: dateMonthPicker.endDate
            when: dateMonthPicker.isCurrent
        }
    }

    DateYearPicker {
        id: dateYearPicker

        property bool isCurrent: periodSelector.currentIndex == 2

        visible: isCurrent

        Binding {
            target: datePeriodPicker
            property: "beginDate"
            value: dateYearPicker.beginDate
            when: dateYearPicker.isCurrent
        }

        Binding {
            target: datePeriodPicker
            property: "endDate"
            value: dateYearPicker.endDate
            when: dateYearPicker.isCurrent
        }
    }

    Item {
        id: dateAllPicker

        property bool isCurrent: periodSelector.currentIndex == 3

        visible: isCurrent

        Binding {
            target: datePeriodPicker
            property: "beginDate"
            value: new Date(0)
            when: dateAllPicker.isCurrent
        }

        Binding {
            target: datePeriodPicker
            property: "endDate"
            value: new Date()
            when: dateAllPicker.isCurrent
        }
    }

    DateRangePicker {
        id: dateRangePicker

        property bool isCurrent: periodSelector.currentIndex == 4

        visible: isCurrent

        Binding {
            target: datePeriodPicker
            property: "beginDate"
            value: dateRangePicker.beginDate
            when: dateRangePicker.isCurrent
        }

        Binding {
            target: datePeriodPicker
            property: "endDate"
            value: dateRangePicker.endDate
            when: dateRangePicker.isCurrent
        }
    }
}
