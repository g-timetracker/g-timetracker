import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Window 2.2

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

            var datePickerPosition = TimeLog.mapToGlobal(datePicker)
            var popupX = datePickerPosition.x
            popupX -= popupX + popup.width > Screen.desktopAvailableWidth ? popup.width - datePicker.width : 0
            var popupY = datePickerPosition.y
            popupY -= popupY - popup.height < 0 ? -datePicker.height : popup.height
            popup.position = Qt.point(popupX, popupY)

            popup.show()
            popup.requestActivate()
        }
    }

    CalendarPopup {
        id: popup
    }
}
