import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQml.Models 2.2
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0
import TimeLog 1.0

ApplicationWindow {
    id: mainWindow

    width: 540
    height: 960
    visible: true

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
                            label: Image {
                                anchors.centerIn: parent
                                source: "images/ic_menu_white_24dp.png"
                            }

                            onClicked: drawer.open()
                        }
                    }

                    Component {
                        id: backButtonComponent

                        ToolButton {
                            text: "back"
                            label: Image {
                                anchors.centerIn: parent
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

    function changeSyncPath() {
        syncPathDialog.open()
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

        anchors.fill: parent

        Pane {
            height: parent.height
            implicitWidth: Math.min(drawerItems.implicitWidth + drawerItems.anchors.margins * 2,
                                    parent.width - 56)

            Flickable {
                anchors.fill: parent
                contentWidth: drawerItems.implicitWidth
                contentHeight: drawerItems.implicitHeight
                boundsBehavior: Flickable.StopAtBounds

                ScrollBar.vertical: ScrollBar { }

                Column {
                    id: drawerItems

                    anchors.margins: 10
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 10

                    PushButton {
                        text: "Recent"
                        onClicked: {
                            drawer.close()
                            mainWindow.showRecent()
                        }
                    }
                    PushButton {
                        text: "Search"
                        onClicked: {
                            drawer.close()
                            mainWindow.showSearch()
                        }
                    }
                    PushButton {
                        text: "Statistics"
                        onClicked: {
                            drawer.close()
                            mainWindow.showStats()
                        }
                    }
                    PushButton {
                        text: "Categories"
                        onClicked: {
                            drawer.close()
                            mainWindow.showCategories()
                        }
                    }
                    PushButton {
                        text: "History"
                        onClicked: {
                            drawer.close()
                            mainWindow.showHistory()
                        }
                    }
                    PushButton {
                        text: "Sync"
                        onClicked: {
                            drawer.close()
                            TimeTracker.syncer.sync(Settings.syncPath)
                        }
                    }
                    PushButton {
                        text: "Undo"
                        enabled: TimeTracker.undoCount
                        onClicked: {
                            drawer.close()
                            TimeTracker.undo()
                        }
                    }
                    LabelControl {
                        font.pixelSize: 16
                        opacity: 1
                        text: "Settings"
                    }
                    Switch {
                        text: "Confirmations"
                        checkable: true
                        checked: Settings.isConfirmationsEnabled
                        onCheckedChanged: Settings.isConfirmationsEnabled = checked
                    }
                    PushButton {
                        text: "Sync path"
                        onClicked: {
                            drawer.close()
                            mainWindow.changeSyncPath()
                        }
                    }
                }
            }
        }

        onClicked: close()
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
        onShowSearchRequested: showSearch(category)
        onShowStatsRequested: showStats(category)
        onShowHistoryRequested: showHistory(begin, end)
        onShowDialogRequested: showDialog(dialog)
        onBackRequested: back()
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

    Item {
        id: mainView

        function pushPage(sourceName, parameters) {
            var page = stackView.push(Qt.resolvedUrl("MobilePage.qml"),
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
            page = stackView.replace(null, isPageExists ? page : Qt.resolvedUrl("MobilePage.qml"),
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
