import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import TimeLog 1.0

Page {
    id: view

    property alias category: timeLogFilter.category

    title: qsTranslate("main window", "Search")

    header: MainToolBarMaterial {
        title: view.title
        isBottomItem: view.StackView.index === 0
    }

    TimeLogSearchModel {
        id: timeLogModel

        timeTracker: TimeTracker
        begin: timeLogFilter.beginDate
        end: timeLogFilter.endDate
        category: timeLogFilter.category
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        TimeLogFilter {
            id: timeLogFilter

            Layout.fillHeight: false
            Layout.fillWidth: true
        }

        TimeLogView {
            id: timeLogView

            function showHistory() {
                var item = timeLogView.currentItem
                var beginDate = new Date(Math.max(item.startTime.valueOf() - 6 * 60 * 60 * 1000, 0))
                var endDate = new Date(Math.min(item.succeedingStart.valueOf() - 1000 + 6 * 60 * 60 * 1000, Date.now()))
                TimeTracker.showHistoryRequested(beginDate, endDate)
            }

            property MenuItem showHistoryMenuItem: MenuItemMaterial {
                text: qsTr("Show in history")
                iconItem.source: "images/ic_history_white_24dp.png"
                onTriggered: timeLogView.showHistory()
            }
            property ItemDelegate showHistoryBottomSheetItem: ItemDelegateMaterial {
                width: timeLogView.width
                text: qsTr("Show in history")
                iconItem.source: "images/ic_history_white_24dp.png"
                onClicked: {
                    timeLogView.showHistory()
                    timeLogView.closeBottomSheet()
                }
            }

            Layout.fillHeight: true
            Layout.fillWidth: true
            model: timeLogModel
            menuModel: [
                showHistoryMenuItem,
                timeLogView.editMenuItem
            ]
            bottomSheetModel: [
                showHistoryBottomSheetItem,
                timeLogView.editBottomSheetItem
            ]
        }
    }
}
