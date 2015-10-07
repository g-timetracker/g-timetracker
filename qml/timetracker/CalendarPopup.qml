import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.4

Window {
    id: popup

    property point position
    property alias minimumDate: calendar.minimumDate
    property alias maximumDate: calendar.maximumDate
    property alias selectedDate: calendar.selectedDate

    x: position.x
    y: position.y - height - 1
    height: calendar.implicitHeight
    width: calendar.implicitWidth
    flags: Qt.Popup

    onFocusObjectChanged: {
        if (!object) {
            hide()
        }
    }

    Calendar {
        id: calendar

        anchors.fill: parent

        onClicked: popup.hide()
    }
}
