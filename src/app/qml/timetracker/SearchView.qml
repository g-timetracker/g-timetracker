import QtQuick 2.4
import QtQuick.Layouts 1.1
import Qt.labs.controls 1.0
import TimeLog 1.0

Item {
    property string title: "Search"
    property alias category: timeLogFilter.category

    TimeLogSearchModel {
        id: timeLogModel

        timeTracker: TimeTracker
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

        TimeLogView {
            id: timeLogView

            property MenuItem showHistoryMenuItem: MenuItem {
                text: "Show history"
                onTriggered: {
                    var item = timeLogView.pointedItem()
                    var beginDate = new Date(Math.max(item.startTime.valueOf() - 6 * 60 * 60 * 1000, 0))
                    var endDate = new Date(Math.min(item.succeedingStart.valueOf() - 1000 + 6 * 60 * 60 * 1000, Date.now()))
                    TimeTracker.showHistoryRequested(beginDate, endDate)
                }
            }

            Layout.fillHeight: true
            Layout.fillWidth: true
            model: timeLogModel
            menuModel: [
                showHistoryMenuItem,
                timeLogView.editMenuItem
            ]
        }
    }
}
