import QtQuick 2.0

ItemPositioner {
    property var beginDate: dateField.selectedDate
    property var endDate: new Date(dateField.selectedDate.valueOf() + 86399000)

    spacing: 10

    ComboBoxControl {
        id: periodSelector


        model: [
            qsTr("current"),
            qsTr("previous"),
            qsTr("select")
        ]

        onCurrentIndexChanged: {
            switch (currentIndex) {
            case 0:
                dateField.selectedDate = new Date(new Date().setHours(0, 0, 0, 0))
                break
            case 1:
                dateField.selectedDate = new Date(new Date(Date.now() - 86400000).setHours(0, 0, 0, 0))
                break
            }
        }
    }

    DatePicker {
        id: dateField

        visible: periodSelector.currentIndex == 2

        minimumDate: new Date(0)
        maximumDate: new Date()
    }
}
