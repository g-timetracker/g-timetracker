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

    Component {
        id: mainToolBar

        ToolBar {
            RowLayout {
                anchors.fill: parent

                Loader {
                    sourceComponent: stackView.depth > 1 ? backButtonComponent : menuButtonComponent

                    Component {
                        id: menuButtonComponent

                        ToolButton {
                            text: "menu"
                            contentItem: Image {
                                fillMode: Image.Pad
                                source: "images/ic_menu_white_24dp.png"
                            }

                            onClicked: drawer.open()
                        }
                    }

                    Component {
                        id: backButtonComponent

                        ToolButton {
                            text: "back"
                            contentItem: Image {
                                fillMode: Image.Pad
                                source: "images/ic_arrow_back_white_24dp.png"
                            }

                            onClicked: mainWindow.back()
                        }
                    }
                }

                LabelControl {
                    Layout.fillWidth: true
                    Material.theme: Material.Dark
                    font.pixelSize: 20
                    opacity: 1
                    text: stackView.currentItem.title ? stackView.currentItem.title : ""
                }
            }

            Rectangle {
                width: parent.width
                implicitHeight: progressBar.implicitHeight
                y: parent.height - height
                color: "white"
                visible: TimeTracker.syncer ? TimeTracker.syncer.isRunning : false

                ProgressBar {
                    id: progressBar

                    width: parent.width
                    indeterminate: true
                }
            }
        }
    }

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
                        TimeTracker.syncer.notifyNextSync = true
                        TimeTracker.syncer.sync(Settings.syncPath)
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

        function pushPage(sourceName, parameters) {
            var page = stackView.push(Qt.resolvedUrl("PageMaterial.qml"),
                                      { "source": sourceName, "toolBar": mainToolBar })
            for (var key in parameters) {
                page.content[key] = parameters[key]
            }
        }

        function switchToPage(pageName, sourceName, parameters) {
            if (stackView.currentItem.objectName === pageName) {
                return
            }

            var page = stackView.find(function (item, index) {
                return item.objectName === pageName
            })
            var isPageExists = !!page
            page = stackView.replace(null, isPageExists ? page : Qt.resolvedUrl("PageMaterial.qml"),
                                     isPageExists ? {} : { "objectName": pageName,
                                                           "source": sourceName,
                                                           "toolBar": mainToolBar })
            if (!isPageExists) {
                for (var key in parameters) {
                    page.content[key] = parameters[key]
                }
            }
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

        Page {
            id: recentPage

            property alias title: recentView.title

            objectName: "recentPage"

            header: Loader {
                sourceComponent: mainToolBar
            }

            RecentView {
                id: recentView

                anchors.fill: parent
            }
        }

        StackView {
            id: stackView

            anchors.fill: parent
            initialItem: recentPage
        }
    }
}
