import QtQuick 2.0
import QtQuick.Controls 1.4

TextField {
    id: datePicker

    property alias minimumDate: popup.minimumDate
    property alias maximumDate: popup.maximumDate
    property alias selectedDate: popup.selectedDate

    function updateDate() {
        selectedDate = Date.fromLocaleDateString(Qt.locale(), text, "yyyy-MM-dd")
    }

    implicitWidth: 140
    text: Qt.formatDate(selectedDate, Qt.ISODate)

    onEditingFinished: updateDate()

    ToolButton {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        iconName: "x-office-calendar"

        onClicked: {
            updateDate()

            popup.showPopup(datePicker)
        }
    }

    CalendarPopup {
        id: popup
    }
}
