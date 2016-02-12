import QtQuick 2.4
import QtQuick.Layouts 1.1
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

            LabelControl {
                id: categoryLabel

                Layout.fillWidth: false
                font.pixelSize: 16
                opacity: 1
                text: timeLogDelegate.category
            }

            LabelControl {
                id: durationTimeLabel

                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideRight
                font.pixelSize: 14
                text: TimeTracker.durationText(timeLogDelegate.durationTime, 2)
            }
        }

        LabelControl {
            id: startLabel

            font.pixelSize: 14
            text: "%1 - %2".arg(Qt.formatDateTime(timeLogDelegate.startTime))
                           .arg(Qt.formatDateTime(new Date(timeLogDelegate.succeedingStart.valueOf() -1000)))
        }

        LabelControl {
            id: commentLabel

            font.pixelSize: 14
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
