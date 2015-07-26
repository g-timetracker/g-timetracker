import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

Item {
    id: timeLogDelegate

    function durationText(duration) {
        var seconds, minutes, hours, days, weeks, months, years

        seconds = duration

        minutes = seconds / 60 | 0
        seconds %= 60

        hours = minutes / 60 | 0
        minutes %= 60

        days = hours / 24 | 0
        hours %= 24

        years = days / 365 | 0
        days %= 365

        months = days / 30 | 0
        days %= 30

        weeks = days / 7 | 0
        days %= 7

        var results = []
        if (years) {
            results.push("%1 years".arg(years))
        }
        if (months) {
            results.push("%1 months".arg(months))
        }
        if (weeks) {
            results.push("%1 weeks".arg(weeks))
        }
        if (days) {
            results.push("%1 days".arg(days))
        }
        if (hours) {
            results.push("%1 hours".arg(hours))
        }
        if (minutes) {
            results.push("%1 minutes".arg(minutes))
        }
        if (seconds) {
            results.push("%1 seconds".arg(seconds))
        }

        return results.join(", ")
    }

    property string category
    property date startTime
    property string durationTime
    property string comment

    width: 400
    implicitHeight: elementsColumn.implicitHeight

    Column {
        id: elementsColumn

        width: parent.width
        spacing: 10

        Label {
            text: "Category: %1".arg(timeLogDelegate.category)
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                id: startLabel

                text: "Start:"
            }

            Label {
                id: startDateLabel

                text: Qt.formatDate(timeLogDelegate.startTime)
            }

            Label {
                id: startTimeLabel

                text: Qt.formatTime(timeLogDelegate.startTime)
            }

            Label {
                id: durationTimeLabel

                Layout.alignment: Qt.AlignRight
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: "Duration: %1".arg(durationText(timeLogDelegate.durationTime))
            }
        }

        Label {
            id: commentLabel

            text: "Comment: %1".arg(timeLogDelegate.comment)
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            visible: !!text
        }
    }
}
