import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import TimeLog 1.0

Item {
    TimeLogSearchModel {
        id: timeLogModel

        begin: new Date(fromField.text)
        end: new Date(toField.text)
        category: categoryField.text
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Item {
            Layout.fillHeight: false
            Layout.fillWidth: true
            implicitHeight: controlsLayout.implicitHeight + controlsLayout.anchors.margins * 2
            implicitWidth: controlsLayout.implicitWidth + controlsLayout.anchors.margins * 2

            GridLayout {
                id: controlsLayout

                anchors.margins: 10
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 10

                Label {
                    text: "From:"
                }

                DatePicker {
                    id: fromField

                    minimumDate: new Date(0)
                    maximumDate: toField.selectedDate
                }

                Label {
                    text: "To:"
                }

                DatePicker {
                    id: toField

                    minimumDate: fromField.selectedDate
                    maximumDate: new Date()
                }

                Label {
                    text: "Category:"
                }

                TextField {
                    id: categoryField
                }
            }
        }

        ListView {
            id: listView

            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            model: timeLogModel
            delegate: TimeLogDelegate {
                width: listView.width
                category: model.category
                startTime: model.startTime
                durationTime: model.durationTime
                comment: model.comment
            }
        }
    }
}
