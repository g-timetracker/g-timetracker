import QtQuick 2.4
import QtQuick.Layouts 1.1
import "ChartColors.js" as ChartColors

Item {
    property alias category: timeLogFilter.category

    Connections {
        target: TimeLog
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
                TimeLog.getStats(beginDate, endDate, category)
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
