import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4

Dialog {
    id: popup

    property alias minimumDate: calendar.minimumDate
    property alias maximumDate: calendar.maximumDate
    property alias selectedDate: calendar.selectedDate

    function showPopup(parentControl) {
        popup.open()
    }

    standardButtons: StandardButton.Cancel

    Calendar {
        id: calendar

        anchors.left: parent.left
        anchors.right: parent.right
        onClicked: popup.close()
    }
}
