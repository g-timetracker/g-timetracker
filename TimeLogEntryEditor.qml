import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3
import "Util.js" as Util

Item {
    id: delegateEditor

    property date startTimeAfter
    property date startTimeBefore
    property var startTimeCurrent: new Date()

    property string category
    property var startTime: new Date()
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
                id: calendar

                property alias origDate: delegateEditor.startTimeCurrent

                minimumDate: startTimeAfter
                maximumDate: startTimeBefore

                onSelectedDateChanged: {
                    var newDate = new Date(selectedDate)
                    delegateEditor.startTime.setFullYear(newDate.getFullYear())
                    delegateEditor.startTime.setMonth(newDate.getMonth())
                    delegateEditor.startTime.setDate(newDate.getDate())
                    delegateEditor.startTime = delegateEditor.startTime
                }

                onOrigDateChanged: selectedDate = origDate
            }

            TimeTumbler {
                id: timeTumbler

                startDateCurrent: calendar.selectedDate
                startTimeCurrent: delegateEditor.startTimeCurrent
                startTimeBefore: delegateEditor.startTimeBefore
                startTimeAfter: delegateEditor.startTimeAfter

                onHoursChanged: {
                    delegateEditor.startTime.setHours(hours)
                    delegateEditor.startTime = delegateEditor.startTime
                }

                onMinutesChanged: {
                    delegateEditor.startTime.setMinutes(minutes)
                    delegateEditor.startTime = delegateEditor.startTime
                }

                onSecondsChanged: {
                    delegateEditor.startTime.setSeconds(seconds)
                    delegateEditor.startTime = delegateEditor.startTime
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
