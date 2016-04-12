import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3
import TimeLog 1.0
import "Util.js" as Util
import "Texts.js" as Texts

GridLayout {
    id: delegateEditor

    property date startTimeAfter
    property date startTimeBefore
    property var startTimeCurrent: new Date()

    property alias category: categoryPicker.category
    property var startTime: new Date()
    property alias comment: commentArea.text

    property bool acceptable: !!category

    QtObject {
        id: d

        property int durationTime: Util.calcDuration(startTime, startTimeBefore)
    }

    columns: 2
    columnSpacing: 10
    rowSpacing: 10

    CategoryPicker {
        id: categoryPicker

        Layout.columnSpan: 2
    }

    LabelControl {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        text: Texts.labelText(qsTranslate("entry editor", "Start date"))
    }

    DatePicker {
        id: calendar

        property alias origDate: delegateEditor.startTimeCurrent

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter

        minimumDate: new Date(startTimeAfter.valueOf() + 1000)
        maximumDate: new Date(startTimeBefore.valueOf() - 1000)

        onOrigDateChanged: selectedDate = origDate
    }

    LabelControl {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        text: Texts.labelText(qsTranslate("entry editor", "Start time"))
    }

    TimeEditor {
        id: timeEditor

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter

        startDateCurrent: calendar.selectedDate
        minDateTime: new Date(startTimeAfter.valueOf() + 1000)
        maxDateTime: new Date(startTimeBefore.valueOf() - 1000)

        onStartTimeCurrentChanged: {
            if (delegateEditor.startTime != startTimeCurrent) {
                delegateEditor.startTime = startTimeCurrent
            }
        }
    }

    LabelControl {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        text: Texts.labelText(qsTranslate("entry editor", "Duration"))
    }

    LabelControl {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        horizontalAlignment: Text.AlignRight
        elide: Text.ElideRight
        text: TimeTracker.durationText(d.durationTime)
    }

    LabelControl {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        text: Texts.labelText(qsTranslate("entry editor", "Comment (optional)"))
    }

    TextArea {
        id: commentArea

        property alias origComment: delegateEditor.comment

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignVCenter
        Layout.columnSpan: delegateEditor.columns

        LabelControl {
            anchors.margins: commentArea.textMargin
            anchors.fill: parent
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            opacity: 0.5
            text: Texts.labelText(qsTranslate("entry editor", "Comment (optional)"))
            visible: !commentArea.text && !commentArea.activeFocus
        }
    }

    onStartTimeCurrentChanged: {
        if (timeEditor.startTimeCurrent != startTimeCurrent) {
            timeEditor.startTimeCurrent = startTimeCurrent
        }
    }
}
