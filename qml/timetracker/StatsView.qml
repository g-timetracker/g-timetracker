import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import jbQuick.Charts 1.0

Item {
    Connections {
        target: TimeLog
        onStatsDataAvailable: {
            if (until.valueOf() !== controlsLayout.endDate.valueOf()) {
                console.log("Discarding stats data for different period, end:", until)
                return
            }

            chart.chartData = {
                labels: [
                    "test"
                ],
                datasets: data
            }
        }
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

                property var beginDate: fromField.selectedDate
                property var endDate: new Date(toField.selectedDate.valueOf() + 86399000)

                function requestStats() {
                    TimeLog.getStats(beginDate, endDate)
                }

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

                onBeginDateChanged: requestStats()
                onEndDateChanged: requestStats()
            }
        }

        Chart {
            id: chart

            Layout.fillHeight: true
            Layout.fillWidth: true
            chartAnimated: false;
            chartAnimationEasing: Easing.Linear;
            chartAnimationDuration: 2000;
            chartType: Charts.ChartType.BAR;
            chartOptions: new Object({
                maintainAspectRatio: false,
            })
        }
    }
}
