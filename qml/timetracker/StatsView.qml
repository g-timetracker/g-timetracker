import QtQuick 2.4
import QtQuick.Layouts 1.1
import TimeLog 1.0
import "ChartColors.js" as ChartColors

Item {
    property alias category: timeLogFilter.category

    Connections {
        target: TimeTracker
        onStatsDataAvailable: {
            if (until.valueOf() !== timeLogFilter.endDate.valueOf()) {
                console.log("Discarding stats data for different period, end:", until)
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

        Chart {
            id: chart

            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
