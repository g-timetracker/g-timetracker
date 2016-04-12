import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import Qt.labs.controls 1.0 as LC
import TimeLog 1.0

ApplicationWindow {
    id: mainWindow

    function showSearch(category) {
        if (mainView.searchIndex === -1) {
            mainView.searchIndex = tabModel.count
            tabModel.append({ "text": "Search", "source": "SearchView.qml",
                                "hasCloseButton": true })
        }
        tabBar.setCurrentIndex(mainView.searchIndex)
        if (category !== undefined) {
            stackLayout.children[mainView.searchIndex].item.category = category
        }
    }

    function showStats(category) {
        if (mainView.statsIndex === -1) {
            mainView.statsIndex = tabModel.count
            tabModel.append({ "text": "Statistics", "source": "StatsView.qml",
                                "hasCloseButton": true })
        }
        tabBar.setCurrentIndex(mainView.statsIndex)
        if (category !== undefined) {
            stackLayout.children[mainView.statsIndex].item.category = category
        }
    }

    function showHistory(beginDate, endDate) {
        if (mainView.historyIndex === -1) {
            mainView.historyIndex = tabModel.count
            tabModel.append({ "text": "History", "source": "HistoryView.qml",
                                "hasCloseButton": true })
        }
        tabBar.setCurrentIndex(mainView.historyIndex)
        if (beginDate !== undefined && endDate !== undefined) {
            stackLayout.children[mainView.historyIndex].item.beginDate = beginDate
            stackLayout.children[mainView.historyIndex].item.endDate = endDate
        }
    }

    function showCategories() {
        if (mainView.categoriesIndex === -1) {
            mainView.categoriesIndex = tabModel.count
            tabModel.append({ "text": "Categories", "source": "CategoriesView.qml",
                                "hasCloseButton": true })
        }
        tabBar.setCurrentIndex(mainView.categoriesIndex)
    }

    function changeSyncPath() {
        syncPathDialog.open()
    }

    function showDialog(dialog) {
        dialog.open()
    }

    width: 640
    height: 480
    visible: true

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTranslate("main window", "Search")
                onTriggered: mainWindow.showSearch()
            }
            MenuItem {
                text: qsTranslate("main window", "Statistics")
                onTriggered: mainWindow.showStats()
            }
            MenuItem {
                text: qsTranslate("main window", "History")
                onTriggered: mainWindow.showHistory()
            }
            MenuItem {
                text: qsTranslate("main window", "Categories")
                onTriggered: mainWindow.showCategories()
            }
            MenuItem {
                text: qsTranslate("main window", "Sync")
                onTriggered: TimeTracker.syncer.sync()
            }
            MenuItem {
                text: qsTranslate("main window", "Undo")
                enabled: TimeTracker.undoCount
                onTriggered: TimeTracker.undo()
            }
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: qsTranslate("main window", "Settings")
            MenuItem {
                text: qsTranslate("settings", "Confirmations")
                checkable: true
                checked: Settings.isConfirmationsEnabled
                onCheckedChanged: Settings.isConfirmationsEnabled = checked
            }
            MenuItem {
                text: qsTranslate("settings", "Sync folder")
                onTriggered: mainWindow.changeSyncPath()
            }
        }
    }

    Binding {
        target: TimeTracker
        property: "dataPath"
        value: TimeLogDataPath ? TimeLogDataPath : Settings.dataPath
    }

    Binding {
        target: TimeTracker.syncer
        property: "syncPath"
        value: TimeLogSyncPath ? TimeLogSyncPath : Settings.syncPath
        when: TimeTracker.syncer
    }

    Binding {
        target: TimeTracker.syncer
        property: "autoSync"
        value: Settings.isAutoSync
        when: TimeTracker.syncer
    }

    Binding {
        target: TimeTracker.syncer
        property: "syncCacheSize"
        value: Settings.syncCacheSize
        when: TimeTracker.syncer
    }

    Connections {
        target: TimeTracker

        onError: {
            errorDialog.text = errorText
            errorDialog.open()
        }
        onShowSearchRequested: showSearch(category)
        onShowStatsRequested: showStats(category)
        onShowHistoryRequested: showHistory(begin, end)
        onShowDialogRequested: showDialog(dialog)
    }

    Connections {
        target: TimeTracker.syncer
        onSynced: {
            messageDialog.text = qsTranslate("main window", "Synced")
            messageDialog.open()
        }
    }

    MessageDialog {
        id: messageDialog

        title: qsTranslate("main window", "Message", "Message dialog title")
        icon: StandardIcon.Information
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: errorDialog

        title: qsTranslate("main window", "Error", "Error dialog title")
        icon: StandardIcon.Critical
        standardButtons: StandardButton.Ok
    }

    FileDialog {
        id: syncPathDialog

        title: qsTranslate("settings", "Select sync folder")
        selectFolder: true

        onAccepted: Settings.syncPath = folder
    }

    Item {
        id: mainView

        property int searchIndex: -1
        property int statsIndex: -1
        property int historyIndex: -1
        property int categoriesIndex: -1

        function clearIndex(index) {
            if (index === statsIndex) {
                statsIndex = -1;
            } else if (index === historyIndex) {
                historyIndex = -1
            } else if (index === categoriesIndex) {
                categoriesIndex = -1
            } else if (index === searchIndex) {
                searchIndex = -1;
            }
        }

        anchors.fill: parent

        ListModel {
            id: tabModel

            ListElement {
                text: QT_TRANSLATE_NOOP("main window", "Recent")
                source: "RecentView.qml"
                hasCloseButton: false
            }
        }

        ColumnLayout {
            anchors.fill: parent

            LC.TabBar {
                id: tabBar

                Layout.fillHeight: false
                Layout.fillWidth: true
                visible: count > 1

                Repeater {
                    model: tabModel
                    delegate: LC.TabButton {
                        text: qsTranslate("main window", model.text)

                        LC.AbstractButton {
                            id: tabCloseButton

                            padding: 4
                            implicitWidth: label.implicitWidth + padding * 2
                            implicitHeight: label.implicitHeight + padding * 2
                            anchors.right: parent.right
                            anchors.top: parent.top
                            visible: model.hasCloseButton
                            text: "x"
                            label: LabelControl {
                                text: tabCloseButton.text
                            }
                            onClicked: {
                                mainView.clearIndex(model.index)
                                tabModel.remove(model.index)
                            }
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
}
