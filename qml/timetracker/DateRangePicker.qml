import QtQuick 2.0
import QtQuick.Controls 1.4

Row {
    property var beginDate: fromField.selectedDate
    property var endDate: new Date(toField.selectedDate.valueOf() + 86399000)

    spacing: 10

    Label {
        anchors.verticalCenter: parent.verticalCenter
        text: "From:"
    }

    DatePicker {
        id: fromField

        anchors.verticalCenter: parent.verticalCenter
        minimumDate: new Date(0)
        maximumDate: toField.selectedDate
    }

    Label {
        anchors.verticalCenter: parent.verticalCenter
        text: "To:"
    }

    DatePicker {
        id: toField

        anchors.verticalCenter: parent.verticalCenter
        minimumDate: fromField.selectedDate
        maximumDate: new Date()
    }
}
