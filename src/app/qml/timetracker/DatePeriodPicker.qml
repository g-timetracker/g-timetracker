import QtQuick 2.0
import QtQuick.Controls 1.4

Row {
    id: datePeriodPicker

    property var beginDate
    property var endDate

    spacing: 10

    ComboBox {
        id: periodSelector

        model: [
            "day",
            "month",
            "year",
            "select..."
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

    DateRangePicker {
        id: dateRangePicker

        property bool isCurrent: periodSelector.currentIndex == 3

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
