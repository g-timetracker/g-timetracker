import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

GridLayout {
    id: controlsLayout

    property var beginDate: datePeriodPicker.beginDate
    property var endDate: datePeriodPicker.endDate
    property string category

    columns: 2
    columnSpacing: 10
    rowSpacing: 10

    DatePeriodPicker {
        id: datePeriodPicker

        Layout.columnSpan: columns
    }

    Label {
        text: "Category:"
    }

    ComboBox {
        id: categoryField

        model: [ "" ].concat(TimeLog.categories)

        onCurrentTextChanged: {
            if (controlsLayout.category !== currentText) {
                controlsLayout.category = currentText
            }
        }
    }

    onCategoryChanged: {
        if (categoryField.currentText !== category) {
            categoryField.currentIndex = categoryField.find(category)
        }
    }
}
