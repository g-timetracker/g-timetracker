import QtQuick 2.0
import QtQuick.Controls 1.4 as QQC1 // FIXME: crashes without this
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

TextField {
    id: datePicker

    property alias minimumDate: popup.minimumDate
    property alias maximumDate: popup.maximumDate
    property alias selectedDate: popup.selectedDate

    implicitWidth: 180
    rightPadding: icon.width + 16
    readOnly: true
    text: Qt.formatDate(selectedDate, "ddd, MMM d yyyy")

    MouseArea {
        anchors.fill: parent

        onClicked: popup.showPopup(datePicker)
    }

    IconMaterial {
        id: icon

        anchors.topMargin: datePicker.topPadding
        anchors.bottomMargin: datePicker.bottomPadding
        anchors.top: datePicker.top
        anchors.bottom: datePicker.bottom
        anchors.right: datePicker.right
        source: "images/ic_arrow_drop_down_white_24dp.png"
    }

    CalendarPopup {
        id: popup
    }
}
