import QtQuick 2.0
import QtQml.Models 2.2
import Qt.labs.controls 1.0
import TimeLog 1.0

ApplicationWindow {
    width: 540
    height: 960
    visible: true

    Drawer {
        id: drawer

        anchors.fill: parent

        Rectangle {
            height: parent.height
            implicitWidth: drawerItems.implicitWidth + drawerItems.anchors.margins * 2

            Column {
                id: drawerItems

                anchors.margins: 10
                anchors.fill: parent
                spacing: 10

                Button {
                    text: "Search"
                    onClicked: {
                        drawer.close()
                        mainView.showSearch()
                    }
                }
                Button {
                    text: "Statistics"
                    onClicked: {
                        drawer.close()
                        mainView.showStats()
                    }
                }
                Button {
                    text: "Categories"
                    onClicked: {
                        drawer.close()
                        mainView.showCategories()
                    }
                }
                Button {
                    text: "History"
                    onClicked: {
                        drawer.close()
                        mainView.showHistory()
                    }
                }
                Button {
                    text: "Sync"
                    onClicked: {
                        drawer.close()
                        mainView.sync(Settings.syncPath)
                    }
                }
                Button {
                    text: "Undo"
                    enabled: TimeTracker.undoCount
                    onClicked: {
                        drawer.close()
                        TimeTracker.undo()
                    }
                }
                Label {
                    text: "Settings"
                }
                Switch {
                    text: "Confirmations"
                    checkable: true
                    checked: Settings.isConfirmationsEnabled
                    onCheckedChanged: Settings.isConfirmationsEnabled = checked
                }
                Button {
                    text: "Sync path"
                    onClicked: {
                        drawer.close()
                        mainView.changeSyncPath()
                    }
                }
            }
        }

        onClicked: close()
    }

    MainView {
        id: mainView

        anchors.fill: parent
    }
}
