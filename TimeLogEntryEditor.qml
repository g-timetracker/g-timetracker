import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3
import "Util.js" as Util

Item {
    id: delegateEditor

    property date startTimeAfter
    property date startTimeBefore

    property string category
    property var startTime
    property string comment

    QtObject {
        id: d

        property int durationTime: (startTimeBefore - startTime) / 1000 |0
    }

    implicitWidth: 600
    implicitHeight: itemsColumn.implicitHeight

    Column {
        id: itemsColumn

        width: parent.width
        spacing: 10

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Category:"
            }

            TextField {
                property alias origCategory: delegateEditor.category

                placeholderText: "Enter category"

                onTextChanged: delegateEditor.category = text

                onOrigCategoryChanged: text = origCategory
            }
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Start:"
            }

            Calendar {
                property alias origDate: delegateEditor.startTime

                minimumDate: startTimeAfter
                maximumDate: startTimeBefore

                onClicked: {
                    var newDate = new Date(date)
                    delegateEditor.startTime.setFullYear(newDate.getFullYear())
                    delegateEditor.startTime.setMonth(newDate.getMonth())
                    delegateEditor.startTime.setDate(newDate.getDate())
                    delegateEditor.startTime = delegateEditor.startTime
                }

                onOrigDateChanged: selectedDate = origDate
            }

            TimeTumbler {
                id: timeTumbler

                property alias origTime: delegateEditor.startTime
                property alias origTimeAfter: delegateEditor.startTimeAfter
                property alias origTimeBefore: delegateEditor.startTimeBefore

                function recalcBoudaries() {
                    if (!startTime) {
                        return
                    }

                    var newMinHours = 0
                    var newMaxHours = 23

                    var newMinMinutes = 0
                    var newMaxMinutes = 59

                    var newMinSeconds = 0
                    var newMaxSeconds = 59

                    if (startTimeAfter.getFullYear() === startTimeBefore.getFullYear()
                               && startTimeAfter.getMonth() === startTimeBefore.getMonth()
                               && startTimeAfter.getDate() === startTimeBefore.getDate()) {
                        newMinHours = startTimeAfter.getHours()
                        newMaxHours = startTimeBefore.getHours()
                        if (newMinHours === newMaxHours) {
                            newMinMinutes = startTimeAfter.getMinutes()
                            newMaxMinutes = startTimeBefore.getMinutes()
                            if (newMaxMinutes === newMaxMinutes){
                                newMinSeconds = startTimeAfter.getSeconds()
                                newMaxSeconds = startTimeBefore.getSeconds()
                            }
                        }
                    } else if (startTime.getFullYear() === startTimeBefore.getFullYear()
                               && startTime.getMonth() === startTimeBefore.getMonth()
                               && startTime.getDate() === startTimeBefore.getDate()) {
                        newMinHours = 0
                        newMaxHours = startTimeBefore.getHours()
                        if (startTime.getHours() === startTimeBefore.getHours()) {
                            newMinMinutes = 0
                            newMaxMinutes = startTimeBefore.getMinutes()
                            if (startTime.getMinutes() === startTimeBefore.getMinutes()) {
                                newMinSeconds = 0
                                newMaxSeconds = startTimeBefore.getSeconds()
                            }
                        }
                    } else if (startTimeAfter.getFullYear() === startTime.getFullYear()
                               && startTimeAfter.getMonth() === startTime.getMonth()
                               && startTimeAfter.getDate() === startTime.getDate()) {
                        newMinHours = startTimeAfter.getHours()
                        newMaxHours = 23
                        if (startTimeAfter.getHours() === startTime.getHours()) {
                            newMinMinutes = startTimeAfter.getMinutes()
                            newMaxMinutes = 59
                            if (startTimeAfter.getMinutes() === startTime.getMinutes()) {
                                newMinSeconds = startTimeAfter.getSeconds()
                                newMaxSeconds = 59
                            }
                        }
                    }

                    if (minHours !== newMinHours || maxHours !== newMaxHours) {
                        minHours = newMinHours
                        maxHours = newMaxHours
                        hoursModel = Util.rangeList(minHours, maxHours + 1)
                    }
                    if (minMinutes !== newMinMinutes || maxMinutes !== newMaxMinutes) {
                        minMinutes = newMinMinutes
                        maxMinutes = newMaxMinutes
                        minutesModel = Util.rangeList(minMinutes, maxMinutes + 1)
                    }
                    if (minSeconds !== newMinSeconds || maxSeconds !== newMaxSeconds) {
                        minSeconds = newMinSeconds
                        maxSeconds = newMaxSeconds
                        secondsModel = Util.rangeList(minSeconds, maxSeconds + 1)
                    }
                }

                onOrigTimeChanged: {
                    recalcBoudaries()
                    hours = origTime.getHours()
                    minutes = origTime.getMinutes()
                    seconds = origTime.getSeconds()
                }
                onOrigTimeAfterChanged: recalcBoudaries()
                onOrigTimeBeforeChanged: recalcBoudaries()

                onHoursChanged: {
                    if (startTime) {
                        startTime.setHours(hours)
                        startTime = startTime
                    }
                }

                onMinutesChanged: {
                    if (startTime) {
                        startTime.setMinutes(minutes)
                        startTime = startTime
                    }
                }

                onSecondsChanged: {
                    if (startTime) {
                        startTime.setSeconds(seconds)
                        startTime = startTime
                    }
                }
            }
        }

        Label {
            width: parent.width
            elide: Text.ElideLeft
            text: "Duration: %1".arg(Util.durationText(d.durationTime))
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Comment (optional)"
            }

            TextField {
                property alias origComment: delegateEditor.comment

                placeholderText: "Optional comment"

                onTextChanged: delegateEditor.comment = text

                onOrigCommentChanged: text = origComment
            }
        }
    }
}
