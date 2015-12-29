import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

GridLayout {
    id: controlsLayout

    property alias beginDate: datePeriodPicker.beginDate
    property alias endDate: datePeriodPicker.endDate
    property alias category: categoryPicker.category

    columns: 2
    columnSpacing: 10
    rowSpacing: 10

    DatePeriodPicker {
        id: datePeriodPicker

        Layout.columnSpan: columns
    }

    CategoryPicker {
        id: categoryPicker
    }
}
