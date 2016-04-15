import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import TimeLog 1.0
import "ChartColors.js" as ChartColors

Item {
    property string title: qsTranslate("main window", "Statistics")
    property alias category: timeLogFilter.category

    Connections {
        target: TimeTracker
        onStatsDataAvailable: {
            if (until.valueOf() !== timeLogFilter.endDate.valueOf()) {
                console.log("Discarding outdated stats data, end:", until, timeLogFilter.endDate)
                return
            }

            chart.units = data.units
            ChartColors.addColors(data.data, "color")
            chart.chartData = data
        }
    }

    ColumnLayout {
        anchors.fill: parent

        TimeLogFilter {
            id: timeLogFilter

            function requestStats() {
                TimeTracker.getStats(beginDate, endDate, category)
            }

            Layout.fillHeight: false
            Layout.fillWidth: true

            onBeginDateChanged: requestStats()
            onEndDateChanged: requestStats()
            onCategoryChanged: requestStats()
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Chart {
                id: chart

                width: parent.width * Screen.devicePixelRatio
                height: parent.height * Screen.devicePixelRatio
                anchors.centerIn: parent
                antialiasing: true
                scale: 1.0 / Screen.devicePixelRatio
                transformOrigin: Item.Center
            }
        }
    }
}
