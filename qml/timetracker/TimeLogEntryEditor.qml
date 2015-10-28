import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3
import "Util.js" as Util

GridLayout {
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

    columns: 2
    columnSpacing: 10
    rowSpacing: 10

    Label {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        text: "Category:"
    }

    ComboBox {
        property alias origCategory: delegateEditor.category

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        editable: true
        model: TimeLog.categories

        onEditTextChanged: delegateEditor.category = editText

        onOrigCategoryChanged: {
            if (editText !== origCategory) {
                currentIndex = find(origCategory)
            }
        }
    }

    Label {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        text: "Start date:"
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

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter

        minimumDate: (dayAfterFit ? startTimeAfter : new Date(startTimeAfter.valueOf() + 1000))
        maximumDate: (dayBeforeFit ? startTimeBefore : new Date(startTimeBefore.valueOf() - 1000))

        onOrigDateChanged: selectedDate = origDate
    }

    Label {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        text: "Start time:"
    }

    TimeEditor {
        id: timeEditor

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter

        startDateCurrent: calendar.selectedDate
        startTimeBefore: delegateEditor.startTimeBefore
        startTimeAfter: delegateEditor.startTimeAfter

        onStartTimeCurrentChanged: {
            if (delegateEditor.startTime != startTimeCurrent) {
                delegateEditor.startTime = startTimeCurrent
            }
        }
    }

    Label {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        text: "Duration:"
    }

    Label {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        horizontalAlignment: Text.AlignRight
        elide: Text.ElideRight
        text: TimeLog.durationText(d.durationTime)
    }

    Label {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        text: "Comment:"
    }

    TextArea {
        id: commentArea

        property alias origComment: delegateEditor.comment

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignVCenter
        Layout.columnSpan: delegateEditor.columns

        onTextChanged: delegateEditor.comment = text

        onOrigCommentChanged: text = origComment

        Label {
            anchors.margins: commentArea.textMargin
            anchors.fill: parent
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            opacity: 0.5
            text: "Optional comment"
            visible: !commentArea.text && !commentArea.activeFocus
        }
    }

    onStartTimeCurrentChanged: {
        if (timeEditor.startTimeCurrent != startTimeCurrent) {
            timeEditor.startTimeCurrent = startTimeCurrent
        }
    }
}
