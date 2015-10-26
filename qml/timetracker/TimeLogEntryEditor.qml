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

        property int durationTime: Util.calcDuration(startTime, startTimeBefore)
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

            ComboBox {
                property alias origCategory: delegateEditor.category

                editable: true
                model: TimeLog.categories

                onEditTextChanged: delegateEditor.category = editText

                onOrigCategoryChanged: {
                    if (editText !== origCategory) {
                        currentIndex = find(origCategory)
                    }
                }
            }
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Start:"
            }

            DatePicker {
                id: calendar

                property alias origDate: delegateEditor.startTimeCurrent

                property bool dayAfterFit: !(startTimeAfter.getHours() === 23
                                             && startTimeAfter.getMinutes() === 59
                                             && startTimeAfter.getSeconds() === 59)
                property bool dayBeforeFit: !(startTimeBefore.getHours() === 0
                                              && startTimeBefore.getMinutes() === 0
                                              && startTimeBefore.getSeconds() === 0)

                minimumDate: (dayAfterFit ? startTimeAfter : new Date(startTimeAfter.valueOf() + 1000))
                maximumDate: (dayBeforeFit ? startTimeBefore : new Date(startTimeBefore.valueOf() - 1000))

                onOrigDateChanged: selectedDate = origDate
            }

            TimeEditor {
                id: timeEditor

                startDateCurrent: calendar.selectedDate
                startTimeBefore: delegateEditor.startTimeBefore
                startTimeAfter: delegateEditor.startTimeAfter

                onStartTimeCurrentChanged: {
                    if (delegateEditor.startTime != startTimeCurrent) {
                        delegateEditor.startTime = startTimeCurrent
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

    onStartTimeCurrentChanged: {
        if (timeEditor.startTimeCurrent != startTimeCurrent) {
            timeEditor.startTimeCurrent = startTimeCurrent
        }
    }
}
