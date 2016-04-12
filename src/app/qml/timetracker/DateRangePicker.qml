import QtQuick 2.0
import "Texts.js" as Texts

ItemPositioner {
    property var beginDate: fromField.selectedDate
    property var endDate: new Date(toField.selectedDate.valueOf() + 86399000)

    spacing: 10

    LabelControl {
        text: Texts.labelText(qsTr("From"))
    }

    DatePicker {
        id: fromField

        minimumDate: new Date(0)
        maximumDate: toField.selectedDate
    }

    LabelControl {
        text: Texts.labelText(qsTr("To"))
    }

    DatePicker {
        id: toField

        minimumDate: fromField.selectedDate
        maximumDate: new Date()
    }
}
