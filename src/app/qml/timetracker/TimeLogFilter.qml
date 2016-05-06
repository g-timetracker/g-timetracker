import QtQuick 2.0

Column {
    property alias beginDate: datePeriodPicker.beginDate
    property alias endDate: datePeriodPicker.endDate
    property alias category: categoryPicker.category

    spacing: 16

    DatePeriodPicker {
        id: datePeriodPicker
    }

    CategoryPicker {
        id: categoryPicker
    }
}
