import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3
import "Util.js" as Util

Item {
    id: delegateEditor

    property alias startTimeAfter: timeTumbler.startTimeAfter
    property alias startTimeBefore: timeTumbler.startTimeBefore

    property string category
    property alias startTime: timeTumbler.startTime
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
                    timeTumbler.startTime.setFullYear(newDate.getFullYear())
                    timeTumbler.startTime.setMonth(newDate.getMonth())
                    timeTumbler.startTime.setDate(newDate.getDate())
                    timeTumbler.startTime = timeTumbler.startTime
                }

                onOrigDateChanged: selectedDate = origDate
            }

            TimeTumbler {
                id: timeTumbler
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
