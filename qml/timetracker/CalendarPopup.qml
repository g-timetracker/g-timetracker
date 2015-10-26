import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.4

Window {
    id: popup

    property alias minimumDate: calendar.minimumDate
    property alias maximumDate: calendar.maximumDate
    property alias selectedDate: calendar.selectedDate

    function showPopup(parentControl) {
        d.positionPopup(parentControl)
        popup.show()
        popup.requestActivate()
    }

    x: d.position.x
    y: d.position.y
    height: calendar.implicitHeight
    width: calendar.implicitWidth
    flags: Qt.Popup

    onFocusObjectChanged: {
        if (!object) {
            hide()
        }
    }

    QtObject {
        id: d

        property point position

        function positionPopup(parentControl) {
            var parentControlPosition = TimeLog.mapToGlobal(parentControl)
            var popupX = parentControlPosition.x
            popupX -= popupX + popup.width > Screen.desktopAvailableWidth ? popup.width - parentControl.width : 0
            var popupY = parentControlPosition.y
            popupY -= popupY - popup.height < 0 ? - parentControl.height : popup.height
            position = Qt.point(popupX, popupY)
        }
    }

    Calendar {
        id: calendar

        anchors.fill: parent

        onClicked: popup.hide()
    }
}
