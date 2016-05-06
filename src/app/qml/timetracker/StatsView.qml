import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0
import "ChartColors.js" as ChartColors

Page {
    id: view

    property alias category: timeLogFilter.category

    title: qsTranslate("main window", "Statistics")

    header: MainToolBarMaterial {
        title: view.title
        isBottomItem: view.StackView.index === 0
        rightText: "customize"
        rightContent: Image {
            fillMode: Image.Pad
            source: "images/ic_tune_white_24dp.png"
        }

        onRightActivated: rightDrawer.open()
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
        spacing: 16

        Label {
            Layout.fillHeight: false
            Layout.fillWidth: true
            topPadding: 16
            leftPadding: 32
            rightPadding: 32
            bottomPadding: 0
            font.pixelSize: 16
            color: Material.hintTextColor
            wrapMode: Text.Wrap
            text: qsTranslate("search filter", "Selected period: %1\u2013%2")
                  .arg(timeLogFilter.beginDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat))
                  .arg(timeLogFilter.endDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat))
        }

        Label {
            Layout.fillHeight: false
            Layout.fillWidth: true
            padding: 0
            leftPadding: 32
            rightPadding: 32
            font.pixelSize: 16
            color: Material.hintTextColor
            wrapMode: Text.Wrap
            visible: !!timeLogFilter.category
            text: qsTranslate("search filter", "Category: \u201C%1\u201D").arg(timeLogFilter.category)
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

    Drawer {
        id: rightDrawer

        height: parent.height + view.header.height
        implicitWidth: Math.min(drawerItems.implicitWidth, parent.width - 56)
        edge: Qt.RightEdge

        Flickable {
            anchors.fill: parent
            contentWidth: drawerItems.implicitWidth
            contentHeight: drawerItems.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar { }

            Column {
                id: drawerItems

                padding: 16
                topPadding: 8
                bottomPadding: 8
                spacing: 16

                TimeLogFilter {
                    id: timeLogFilter

                    function requestStats() {
                        TimeTracker.getStats(beginDate, endDate, category)
                    }

                    onBeginDateChanged: requestStats()
                    onEndDateChanged: requestStats()
                    onCategoryChanged: requestStats()
                }

                LabelControl {
                    text: qsTr("Units")
                }

                ComboBoxControl {
                    id: unitSelector

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

            }
        }
    }
}
