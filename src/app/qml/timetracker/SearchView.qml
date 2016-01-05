import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import TimeLog 1.0

Item {
    property alias category: timeLogFilter.category

    TimeLogSearchModel {
        id: timeLogModel

        timeTracker: TimeTracker
        begin: timeLogFilter.beginDate
        end: timeLogFilter.endDate
        category: timeLogFilter.category
    }

    Action {
        id: showHistoryAction

        text: "Show history"
        tooltip: "Show history with this item"

        onTriggered: {
            var item = timeLogView.pointedItem()
            var beginDate = new Date(Math.max(item.startTime.valueOf() - 6 * 60 * 60 * 1000, 0))
            var endDate = new Date(Math.min(item.succeedingStart.valueOf() - 1000 + 6 * 60 * 60 * 1000, Date.now()))
            MainWindow.showHistory(beginDate, endDate)
        }
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

            Layout.fillHeight: true
            Layout.fillWidth: true
            model: timeLogModel
            menu: Menu {
                MenuItem {
                    action: showHistoryAction
                }
                MenuItem {
                    action: timeLogView.editAction
                }
            }
        }
    }
}
