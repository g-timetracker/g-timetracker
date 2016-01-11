import QtQuick 2.0
import QtQuick.Controls 1.4
import "Texts.js" as Texts

ItemPositioner {
    property var beginDate: fromField.selectedDate
    property var endDate: new Date(toField.selectedDate.valueOf() + 86399000)

    spacing: 10

    Label {
        text: Texts.labelText("From")
    }

    DatePicker {
        id: fromField

        minimumDate: new Date(0)
        maximumDate: toField.selectedDate
    }

    Label {
        text: Texts.labelText("To")
    }

    DatePicker {
        id: toField

        minimumDate: fromField.selectedDate
        maximumDate: new Date()
    }
}
