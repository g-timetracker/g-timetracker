import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
import "Util.js" as Util

Item {
    id: timeLogDelegate

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
                text: "Duration: %1".arg(Util.durationText(timeLogDelegate.durationTime))
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
