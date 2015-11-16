import QtQuick 2.0
import QtQuick.Controls 1.4

Row {
    property var beginDate: dateField.selectedDate
    property var endDate: new Date(dateField.selectedDate.valueOf() + 86399000)

    spacing: 10

    ComboBox {
        id: periodSelector

        model: [
            "current",
            "previous",
            "select..."
        ]

        onCurrentIndexChanged: {
            switch (currentIndex) {
            case 0:
                dateField.selectedDate = new Date()
                break
            case 1:
                dateField.selectedDate = new Date(Date.now() - 86400000)
                break
            }
        }
    }

    DatePicker {
        id: dateField

        visible: periodSelector.currentIndex == 2

        anchors.verticalCenter: parent.verticalCenter
        minimumDate: new Date(0)
        maximumDate: new Date()
    }
}
