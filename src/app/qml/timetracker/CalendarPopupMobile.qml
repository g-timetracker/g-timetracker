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

    property bool isLandscape: ApplicationWindow.window.width > ApplicationWindow.window.height

    function showPopup(parentControl) {
        popup.open()
    }

    onSelectedDateChanged: {
        calendarList.selectedDate = selectedDate
        calendarList.positionOnSelectedDate()
    }

    onAccepted: selectedDate = calendarList.selectedDate

    contentItem: Grid {
        rows: isLandscape ? 1 : 2
        columns: isLandscape ? 2 : 1
        width: isLandscape ? 512 : 328
        height: isLandscape ? 252 + buttonBox.height : 96 + 336 + buttonBox.height

        Rectangle {
            width: isLandscape ? 168 : parent.width
            height: isLandscape ? parent.height : 96
            color: popup.Material.accentColor

            Column {
                width: parent.width

                Label {
                    topPadding: isLandscape ? 18 : 20
                    bottomPadding: isLandscape ? 0 : 2
                    leftPadding: isLandscape ? 16 : 24
                    rightPadding: isLandscape ? 16 : 24
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 16
                    color: popup.Material.dialogColor
                    text: calendarList.selectedDate.getFullYear()
                }

                Label {
                    leftPadding: isLandscape ? 14 : 24
                    rightPadding: isLandscape ? 14 : 24
                    width: parent.width
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 34
                    font.weight: Font.Medium
                    color: popup.Material.dialogColor
                    wrapMode: Text.Wrap
                    text: calendarList.selectedDate.toLocaleDateString(Qt.locale(), "ddd, MMM d")
                }
            }
        }

        Column {
            width: isLandscape ? 344 : parent.width

            ListView {
                id: calendarList

                property date selectedDate

                function positionOnSelectedDate() {
                    currentIndex = calendarModel.indexOf(selectedDate)
                }

                implicitHeight: isLandscape ? 252 : 336
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
                        topPadding: isLandscape ? 20 : 18
                        bottomPadding: isLandscape ? 8 : 20
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
                            padding: 0
                            bottomPadding: isLandscape ? 4 : 10
                            width: isLandscape ? 46 : 44
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
                        spacing: isLandscape ? 20 : 0
                        padding: 0
                        topPadding: isLandscape ? 0 : 6
                        locale: popup.locale
                        delegate: Label {
                            property bool isSelected: model.date.valueOf() === calendarList.selectedDate.valueOf()

                            height: isLandscape ? 12 : 40
                            width: isLandscape ? 26 : 44
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
                                width: isLandscape ? 32 : 40
                                height: width
                                anchors.centerIn: parent
                                radius: width / 2
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

            DialogMobileButtonBox {
                id: buttonBox

                width: parent.width
                affirmativeText: popup.affirmativeText
                dismissiveText: popup.dismissiveText

                onAccepted: popup.accept()
                onRejected: popup.reject()
            }
        }
    }
}
