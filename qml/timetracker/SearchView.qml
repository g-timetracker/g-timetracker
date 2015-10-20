import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import TimeLog 1.0
import "Util.js" as Util

Item {
    property alias category: timeLogFilter.category

    TimeLogSearchModel {
        id: timeLogModel

        begin: timeLogFilter.beginDate
        end: timeLogFilter.endDate
        category: timeLogFilter.category
    }

    ColumnLayout {
        anchors.fill: parent

        TimeLogFilter {
            id: timeLogFilter

            Layout.fillHeight: false
            Layout.fillWidth: true
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
