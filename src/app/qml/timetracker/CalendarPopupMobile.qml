import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import Qt.labs.controls 1.0
import Qt.labs.calendar 1.0

Dialog {
    id: popup

    width: 500
    height: 300

    property alias minimumDate: calendarModel.from
    property alias maximumDate: calendarModel.to
    property var selectedDate

    function showPopup(parentControl) {
        popup.open()
    }

    standardButtons: StandardButton.Cancel

    onSelectedDateChanged: calendarList.currentIndex = calendarModel.indexOf(selectedDate)

    ListView {
        id: calendarList

        width: 500 - 2 * 16
        height: 300
        snapMode: ListView.SnapOneItem
        orientation: ListView.Horizontal
        highlightRangeMode: ListView.StrictlyEnforceRange
        clip: true

        model: calendarModel
        delegate: ColumnLayout {
            width: calendarList.width
            height: calendarList.height

            Label {
                Layout.fillHeight: false
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: monthGrid.title
            }

            DayOfWeekRow {
                Layout.fillHeight: false
                Layout.fillWidth: true
                locale: monthGrid.locale
            }

            MonthGrid {
                id: monthGrid

                Layout.fillHeight: true
                Layout.fillWidth: true
                month: model.month
                year: model.year

                onClicked: {
                    if (date >= calendarModel.from && date <= calendarModel.to) {
                        popup.selectedDate = date
                        popup.close()
                    }
                }
            }
        }

        CalendarModel {
            id: calendarModel

            from: new Date(0)
            to: new Date()
        }
    }
}
