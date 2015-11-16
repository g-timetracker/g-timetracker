import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
import TimeLog 1.0

Item {
    id: timeLogDelegate

    property string category
    property date startTime
    property int durationTime
    property string comment
    property date precedingStart
    property date succeedingStart
    property bool isLastItem: false

    function updateData(category, startTime, comment) {
        if (timeLogDelegate.category !== category) {
            model.category = category
        }
        if (timeLogDelegate.startTime.valueOf() !== startTime.valueOf()) {
            model.startTime = startTime
        }

        if (timeLogDelegate.comment !== comment) {
            model.comment = comment
        }
    }

    width: 400
    implicitHeight: elementsColumn.implicitHeight + elementsColumn.spacing

    Column {
        id: elementsColumn

        anchors.margins: spacing
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 10

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                id: categoryLabel

                Layout.fillWidth: false
                font.pointSize: 12
                font.bold: true
                text: timeLogDelegate.category
            }

            Label {
                id: durationTimeLabel

                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideRight
                font.pointSize: 10
                text: TimeTracker.durationText(timeLogDelegate.durationTime, 2)
            }
        }

        Label {
            id: startLabel

            font.pointSize: 10
            text: "%1 - %2".arg(Qt.formatDateTime(timeLogDelegate.startTime))
                           .arg(Qt.formatDateTime(new Date(timeLogDelegate.succeedingStart.valueOf() -1000)))
        }

        Label {
            id: commentLabel

            font.pointSize: 9
            opacity: 0.6
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: timeLogDelegate.comment
            visible: !!text
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "black"
            opacity: isLastItem ? 0 : 0.12
        }
    }
}
