import QtQuick 2.0
import QtQuick.Controls 1.4

ItemPositioner {
    property var beginDate: fromField.selectedDate
    property var endDate: new Date(toField.selectedDate.valueOf() + 86399000)

    spacing: 10

    Label {
        text: "From:"
    }

    DatePicker {
        id: fromField

        minimumDate: new Date(0)
        maximumDate: toField.selectedDate
    }

    Label {
        text: "To:"
    }

    DatePicker {
        id: toField

        minimumDate: fromField.selectedDate
        maximumDate: new Date()
    }
}
