import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import Qt.labs.controls 1.0
import TimeLog 1.0

Item {
    id: mainView

    function showSearch(category) {
        if (d.searchIndex === -1) {
            d.searchIndex = tabModel.count
            tabModel.append({ "text": "Search", "source": "SearchView.qml",
                                "hasCloseButton": true })
            stackLayout.children[d.searchIndex].item.mainView = mainView
        }
        tabBar.setCurrentIndex(d.searchIndex)
        if (category !== undefined) {
            stackLayout.children[d.searchIndex].item.category = category
        }
    }

    function showStats(category) {
        if (d.statsIndex === -1) {
            d.statsIndex = tabModel.count
            tabModel.append({ "text": "Statistics", "source": "StatsView.qml",
                                "hasCloseButton": true })
        }
        tabBar.setCurrentIndex(d.statsIndex)
        if (category !== undefined) {
            stackLayout.children[d.statsIndex].item.category = category
        }
    }

    function showHistory(beginDate, endDate) {
        if (d.historyIndex === -1) {
            d.historyIndex = tabModel.count
            tabModel.append({ "text": "History", "source": "HistoryView.qml",
                                "hasCloseButton": true })
        }
        tabBar.setCurrentIndex(d.historyIndex)
        if (beginDate !== undefined && endDate !== undefined) {
            stackLayout.children[d.historyIndex].item.beginDate = beginDate
            stackLayout.children[d.historyIndex].item.endDate = endDate
        }
    }

    function showCategories() {
        if (d.categoriesIndex === -1) {
            d.categoriesIndex = tabModel.count
            tabModel.append({ "text": "Categories", "source": "CategoriesView.qml",
                                "hasCloseButton": true })
            stackLayout.children[d.categoriesIndex].item.mainView = mainView
        }
        tabBar.setCurrentIndex(d.categoriesIndex)
    }

    function changeSyncPath() {
        syncPathDialog.open()
    }

    function sync(syncPath) {
        TimeTracker.syncer.sync(syncPath)
    }

    QtObject {
        id: d

        property int searchIndex: -1
        property int statsIndex: -1
        property int historyIndex: -1
        property int categoriesIndex: -1
    }

    Binding {
        target: TimeTracker
        property: "dataPath"
        value: TimeLogDataPath ? TimeLogDataPath : Settings.dataPath
    }

    Connections {
        target: TimeTracker

        onError: {
            errorDialog.text = errorText
            errorDialog.open()
        }
    }

    Connections {
        target: TimeTracker.syncer
        onSynced: {
            messageDialog.text = "Sync complete"
            messageDialog.open()
        }
    }

    MessageDialog {
        id: messageDialog

        title: "Message"
        icon: StandardIcon.Information
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: errorDialog

        title: "Error"
        icon: StandardIcon.Critical
        standardButtons: StandardButton.Ok
    }

    FileDialog {
        id: syncPathDialog

        title: "Select folder for sync"
        selectFolder: true

        onAccepted: Settings.syncPath = folder
    }

    ListModel {
        id: tabModel

        ListElement {
            text: "Recent entries"
            source: "RecentView.qml"
            hasCloseButton: false
        }
    }

    ColumnLayout {
        anchors.fill: parent

        TabBar {
            id: tabBar

            Layout.fillHeight: false
            Layout.fillWidth: true
            visible: count > 1

            Repeater {
                model: tabModel
                delegate: TabButton {
                    text: model.text

                    ToolButton {
                        anchors.right: parent.right
                        visible: model.hasCloseButton
                        text: "x"
                        onClicked: tabModel.remove(model.index)
                    }
                }
            }
        }

        StackLayout {
            id: stackLayout

            Layout.fillHeight: true
            Layout.fillWidth: true
            currentIndex: tabBar.currentIndex

            Repeater {
                model: tabModel
                delegate: Loader {
                    source: model.source
                }
            }
        }
    }
}
