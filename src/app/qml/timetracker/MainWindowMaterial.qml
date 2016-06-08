/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQml.Models 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

ApplicationWindow {
    id: mainWindow

    visible: true
    title: Qt.application.name

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

    function showMessage(message) {
        messageDialog.text = message
        messageDialog.open()
    }

    function sync() {
        if (!TimeTracker.syncer.syncPath.toString()) {
            if (!PlatformMaterial.isDesktop) {
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
            TimeTracker.syncer.sync(AppSettings.syncPath)
        }
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
                    text: qsTranslate("main window", "Categories")
                    iconItem.source: "images/ic_folder_white_24dp.png"
                    onClicked: {
                        drawer.close()
                        mainWindow.showCategories()
                    }
                }
                Item {
                    Layout.fillWidth: true
                    height: 16
                    Rectangle {
                        width: parent.width
                        height: 1
                        y: parent.height / 2 - height
                        color: mainWindow.Material.dividerColor
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
        value: !!TimeLogDataPath.toString() ? TimeLogDataPath : AppSettings.dataPath
    }

    Binding {
        target: TimeTracker.syncer
        property: "syncPath"
        value: !!TimeLogSyncPath.toString() ? TimeLogSyncPath : AppSettings.syncPath
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
        value: AppSettings.isAutoSync
        when: TimeTracker.syncer
    }

    Binding {
        target: TimeTracker.syncer
        property: "syncCacheSize"
        value: AppSettings.syncCacheSize
        when: TimeTracker.syncer
    }

    Binding {
        target: TimeTracker.syncer
        property: "syncCacheTimeout"
        value: AppSettings.syncCacheTimeout
        when: TimeTracker.syncer
    }

    Binding {
        target: PlatformMaterial
        property: "applicationWindow"
        value: mainWindow
    }

    Connections {
        target: TimeTracker

        onError: mainWindow.showMessage(errorText)
        onShowSearchRequested: showSearch(category)
        onShowStatsRequested: showStats(category)
        onShowHistoryRequested: showHistory(begin, end)
        onShowDialogRequested: showDialog(dialog)
        onSyncRequested: mainWindow.sync()
        onOpenNavigationDrawerRequested: drawer.open()
        onBackRequested: back()
        onActivateRequested: {
            mainWindow.show()
            mainWindow.raise()
            mainWindow.requestActivate()
        }
    }

    Connections {
        target: TimeTracker.syncer
        onSynced: mainWindow.showMessage(qsTranslate("main window", "Synced"))
    }

    AlertDialog {
        id: messageDialog

        onLinkActivated: Qt.openUrlExternally(link)
    }

    FocusScope {
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

    Component.onCompleted: {
        if (!AppSettings.lastVersion) {    // First start
            AppSettings.lastVersion = Qt.application.version
        }
    }
}
