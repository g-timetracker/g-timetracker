import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

Column {
    id: timeLogDelegate

    property string category
    property string startTime
    property string durationTime
    property string comment

    width: 400
    spacing: 10

    Label {
        id: categoryLabel

        text: category
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

            text: Qt.formatDate(startTime)
        }

        Label {
            id: startTimeLabel

            text: Qt.formatTime(startTime)
        }

        Label {
            id: durationTimeLabel

            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: "Duration: %1".arg(durationTime)
        }
    }

    Label {
        id: commentLabel

        text: "Comment: %1".arg(comment)
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        visible: !!text
    }
}

