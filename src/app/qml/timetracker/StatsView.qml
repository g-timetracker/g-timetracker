import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import TimeLog 1.0
import "ChartColors.js" as ChartColors

Page {
    id: view

    property alias category: timeLogFilter.category

    title: qsTranslate("main window", "Statistics")

    header: MainToolBarMaterial {
        title: view.title
        isBottomItem: view.StackView.index === 0
    }

    function setChartUnits(data) {
        if (unitSelector.currentIndex === 0) {
            if (chart.unitsValue === 3600) {
                return
            }
            chart.unitsValue = 3600
            chart.unitsName = "%n hr"
        } else {
            if (chart.unitsValue === data.unitsValue) {
                return
            }
            chart.unitsValue = data.unitsValue
            chart.unitsName = data.unitsName
        }
    }

    Connections {
        target: TimeTracker
        onStatsDataAvailable: {
            if (until.valueOf() !== timeLogFilter.endDate.valueOf()) {
                console.log("Discarding outdated stats data, end:", until, timeLogFilter.endDate)
                return
            }

            setChartUnits(data)
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

        LabelControl {
            text: qsTr("Units")
        }

        ComboBoxControl {
            id: unitSelector

            Layout.fillHeight: false
            Layout.fillWidth: false
            model: [
                qsTranslate("duration", "hours"),
                qsTranslate("duration", "auto")
            ]

            onCurrentIndexChanged: {
                if (!chart.chartData) {
                    return
                }

                setChartUnits(chart.chartData)

                chart.updateChart()
            }
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
