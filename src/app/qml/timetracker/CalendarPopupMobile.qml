import QtQuick 2.0
import QtQuick.Layouts 1.3
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0
import Qt.labs.calendar 1.0

DialogMobile {
    id: popup

    customTitleBar: true

    property alias minimumDate: calendarModel.from
    property alias maximumDate: calendarModel.to
    property var selectedDate

    function showPopup(parentControl) {
        popup.open()
    }

    onSelectedDateChanged: {
        calendarList.selectedDate = selectedDate
        calendarList.positionOnSelectedDate()
    }

    onAccepted: selectedDate = calendarList.selectedDate

    Column {
        width: 328

        Rectangle {
            width: parent.width
            height: 96
            color: popup.Material.accentColor

            Column {
                width: parent.width

                Label {
                    topPadding: 20
                    bottomPadding: 2
                    leftPadding: 24
                    rightPadding: 24
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 16
                    color: popup.Material.dialogColor
                    text: calendarList.selectedDate.getFullYear()
                }

                Label {
                    leftPadding: 24
                    rightPadding: 24
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 34
                    font.weight: Font.Medium
                    color: popup.Material.dialogColor
                    wrapMode: Text.Wrap
                    text: calendarList.selectedDate.toLocaleDateString(Qt.locale(), "ddd, MMM d")
                }
            }
        }

        ListView {
            id: calendarList

            property date selectedDate

            function positionOnSelectedDate() {
                currentIndex = calendarModel.indexOf(selectedDate)
            }

            implicitHeight: 336
            width: parent.width
            snapMode: ListView.SnapOneItem
            orientation: ListView.Horizontal
            highlightRangeMode: ListView.StrictlyEnforceRange
            clip: true

            model: calendarModel
            delegate: Column {
                width: calendarList.width
                height: calendarList.height

                Label {
                    height: 56
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font: popup.font
                    color: popup.Material.primaryTextColor
                    text: monthGrid.title
                }

                DayOfWeekRow {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 0
                    padding: 0
                    locale: monthGrid.locale
                    font: popup.font
                    delegate: Label {
                        height: 16
                        width: 44
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 12
                        color: popup.Material.hintTextColor
                        text: model.narrowName
                    }
                }

                MonthGrid {
                    id: monthGrid

                    anchors.horizontalCenter: parent.horizontalCenter
                    month: model.month
                    year: model.year
                    font: popup.font
                    spacing: 0
                    padding: 0
                    topPadding: 6
                    delegate: Label {
                        property bool isSelected: model.date.valueOf() === calendarList.selectedDate.valueOf()

                        height: 40
                        width: 44
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 12
                        opacity: model.month === monthGrid.month ? 1 : 0
                        color: {
                            if (isSelected) {
                                return popup.Material.dialogColor
                            } else if (model.today) {
                                return popup.Material.accentColor
                            } else if (date >= calendarModel.from && date <= calendarModel.to) {
                                return popup.Material.primaryTextColor
                            } else {
                                return popup.Material.hintTextColor
                            }
                        }
                        text: model.day

                        background: Rectangle {
                            width: 40
                            height: 40
                            anchors.centerIn: parent
                            radius: 20
                            color: popup.Material.accentColor
                            visible: isSelected
                        }
                    }

                    onClicked: {
                        if (date >= calendarModel.from && date <= calendarModel.to) {
                            calendarList.selectedDate = date
                        }
                    }
                }
            }

            CalendarModel {
                id: calendarModel

                from: new Date(0)
                to: new Date()

                onFromChanged: calendarList.positionOnSelectedDate()
                onToChanged: calendarList.positionOnSelectedDate()
            }
        }
    }
}
