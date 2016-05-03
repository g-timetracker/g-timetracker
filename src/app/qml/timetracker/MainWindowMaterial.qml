import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQml.Models 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

ApplicationWindow {
    id: mainWindow

    width: 540
    height: 960
    visible: true

    Material.primary: Material.BlueGrey
    Material.accent: Material.Cyan

    function showRecent() {
        if (stackView.currentItem != recentPage) {
            stackView.replace(null, recentPage)
        }
    }

    function showSearch(category) {
        if (category) {
            mainView.pushPage("SearchView.qml", { "category": category })
        } else {
            mainView.switchToPage("searchPage", "SearchView.qml")
        }
    }

    function showStats(category) {
        if (category) {
            mainView.pushPage("StatsView.qml", { "category": category })
        } else {
            mainView.switchToPage("statsPage", "StatsView.qml")
        }
    }

    function showHistory(beginDate, endDate) {
        if (beginDate || endDate) {
            mainView.pushPage("HistoryView.qml", { "beginDate": beginDate, "endDate": endDate })
        } else {
            mainView.switchToPage("historyPage", "HistoryView.qml")
        }
    }

    function showCategories() {
        mainView.switchToPage("categoriesPage", "CategoriesView.qml")
    }

    function showSettings() {
        mainView.pushPage("SettingsMaterial.qml")
    }

    function showDialog(dialog) {
        stackView.push(dialog)
    }

    function back() {
        if (stackView.depth > 1) {
            stackView.pop()
        } else {
            mainWindow.showRecent()
        }
    }

    Drawer {
        id: drawer

        height: parent.height
        implicitWidth: Math.min(drawerItems.implicitWidth + drawerItems.anchors.margins * 2,
                                parent.width - 56)

        Flickable {
            anchors.fill: parent
            contentWidth: drawerItems.implicitWidth
            contentHeight: drawerItems.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar { }

            ColumnLayout {
                id: drawerItems

                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0

                ItemDelegateMaterial {
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    text: qsTranslate("main window", "Recent")
                    iconItem.source: "images/ic_today_white_24dp.png"
                    onClicked: {
                        drawer.close()
                        mainWindow.showRecent()
                    }
                }
                ItemDelegateMaterial {
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    text: qsTranslate("main window", "Search")
                    iconItem.source: "images/ic_search_white_24dp.png"
                    onClicked: {
                        drawer.close()
                        mainWindow.showSearch()
                    }
                }
                ItemDelegateMaterial {
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    text: qsTranslate("main window", "Statistics")
                    iconItem.source: "images/ic_show_chart_white_24dp.png"
                    onClicked: {
                        drawer.close()
                        mainWindow.showStats()
                    }
                }
                ItemDelegateMaterial {
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    text: qsTranslate("main window", "Categories")
                    iconItem.source: "images/ic_folder_white_24dp.png"
                    onClicked: {
                        drawer.close()
                        mainWindow.showCategories()
                    }
                }
                ItemDelegateMaterial {
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    text: qsTranslate("main window", "History")
                    iconItem.source: "images/ic_history_white_24dp.png"
                    onClicked: {
                        drawer.close()
                        mainWindow.showHistory()
                    }
                }
                ItemDelegateMaterial {
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    text: qsTranslate("main window", "Sync")
                    iconItem.source: "images/ic_sync_white_24dp.png"
                    onClicked: {
                        drawer.close()

                        if (!TimeTracker.syncer.syncPath.toString()) {
                            if (Qt.platform.os === "android") {
                                mainView.pushPages(
                                            [
                                                {
                                                    "sourceName": "SettingsMaterial.qml",
                                                },
                                                {
                                                    "sourceName": "SyncFolderDialogMaterial.qml",
                                                    "parameters": {
                                                        "folder": TimeTracker.documentsLocation()
                                                    }
                                                }
                                            ])
                            } else {
                                mainWindow.showSettings()
                                stackView.currentItem.openSyncFolderDialog()
                            }
                        } else {
                            TimeTracker.syncer.notifyNextSync = true
                            TimeTracker.syncer.sync(Settings.syncPath)
                        }
                    }
                }
                ItemDelegateMaterial {
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    text: qsTranslate("main window", "Undo")
                    iconItem.source: "images/ic_undo_white_24dp.png"
                    enabled: TimeTracker.undoCount
                    onClicked: {
                        drawer.close()
                        TimeTracker.undo()
                    }
                }
                ItemDelegateMaterial {
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    text: qsTranslate("main window", "Settings")
                    iconItem.source: "images/ic_settings_white_24dp.png"
                    onClicked: {
                        drawer.close()
                        mainWindow.showSettings()
                    }
                }
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
        property: "notifySync"
        value: false
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

    Binding {
        target: TimeTracker.syncer
        property: "syncCacheTimeout"
        value: Settings.syncCacheTimeout
        when: TimeTracker.syncer
    }

    Connections {
        target: TimeTracker

        onError: {
            messageDialog.text = errorText
            messageDialog.open()
        }
        onShowSearchRequested: showSearch(category)
        onShowStatsRequested: showStats(category)
        onShowHistoryRequested: showHistory(begin, end)
        onShowDialogRequested: showDialog(dialog)
        onOpenNavigationDrawerRequested: drawer.open()
        onBackRequested: back()
    }

    Connections {
        target: TimeTracker.syncer
        onSynced: {
            messageDialog.text = qsTranslate("main window", "Synced")
            messageDialog.open()
        }
    }

    AlertDialog {
        id: messageDialog
    }

    Item {
        id: mainView

        function pushPages(pages) {
            var stackPages = []
            for (var i = 0; i < pages.length; i++) {
                stackPages.push(Qt.resolvedUrl(pages[i]["sourceName"]))
                stackPages.push(pages[i]["parameters"] || {})
            }

            stackView.push(stackPages)
        }

        function pushPage(sourceName, parameters) {
            pushPages([{
                           "sourceName": sourceName,
                           "parameters": parameters
                      }])
        }

        function switchToPage(pageName, sourceName) {
            if (stackView.currentItem.objectName === pageName) {
                return
            }

            var page = stackView.find(function (item, index) {
                return item.objectName === pageName
            })
            stackView.replace(null, !!page ? page : Qt.resolvedUrl(sourceName),
                              !!page ? {} : { "objectName": pageName })
        }

        anchors.fill: parent
        focus: true

        Keys.onBackPressed: {
            if (drawer.position) {
                drawer.close()
            } else {
                mainWindow.back()
            }
        }

        RecentView {
            id: recentPage

            objectName: "recentPage"
        }

        StackView {
            id: stackView

            anchors.fill: parent
            initialItem: recentPage
        }
    }
}
