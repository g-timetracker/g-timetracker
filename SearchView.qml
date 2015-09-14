import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import TimeLog 1.0
import "Util.js" as Util

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

        TableView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            model: timeLogModel

            TableViewColumn {
                title: "Start time"
                role: "startTime"
                delegate: Text {
                    color: styleData.textColor
                    elide: styleData.elideMode
                    text: Qt.formatDateTime(styleData.value)
                    renderType: Text.NativeRendering
                }
            }

            TableViewColumn {
                title: "Category"
                role: "category"
            }

            TableViewColumn {
                title: "Duration"
                role: "durationTime"
                delegate: Text {
                    color: styleData.textColor
                    elide: styleData.elideMode
                    text: Util.durationText(styleData.value)
                    renderType: Text.NativeRendering
                }
            }

            TableViewColumn {
                title: "Comment"
                role: "comment"
            }
        }
    }
}
