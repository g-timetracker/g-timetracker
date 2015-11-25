import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import TimeLog 1.0

Item {
    id: categoryView

    property MainView mainView

    Dialog {
        id: renameDialog

        property string categoryName

        function setData() {
            textField.text = categoryName
        }

        function openDialog(category) {
            renameDialog.categoryName = category
            setData()
            open()
        }

        standardButtons: StandardButton.Save | StandardButton.Cancel | StandardButton.Reset
        title: "Rename category"

        TextField {
            id: textField

            anchors.left: parent.left
            anchors.right: parent.right
        }

        onAccepted: {
            if (textField.text !== categoryName) {
                TimeTracker.editCategory(categoryName, textField.text)
            }
        }

        onReset: setData()
    }

    Action {
        id: renameAction

        text: "Rename"
        tooltip: "Rename category"
        enabled: tableView.currentRow !== -1

        onTriggered: renameDialog.openDialog(tableView.model[tableView.currentRow])
    }

    Action {
        id: showEntriesAction

        text: "Show entries"
        tooltip: "Show entries for this category"
        enabled: tableView.currentRow !== -1

        onTriggered: categoryView.mainView.showSearch((tableView.model[tableView.currentRow]))
    }

    Action {
        id: showStatsAction

        text: "Show statistics"
        tooltip: "Show statistics for this category"
        enabled: tableView.currentRow !== -1

        onTriggered: categoryView.mainView.showStats((tableView.model[tableView.currentRow]))
    }

    Menu {
        id: itemMenu

        MenuItem {
            action: renameAction
        }
        MenuItem {
            action: showEntriesAction
        }
        MenuItem {
            action: showStatsAction
        }
    }

    ColumnLayout {
        anchors.fill: parent

        TableView {
            id: tableView

            Layout.fillWidth: true
            Layout.fillHeight: true
            model: TimeTracker.categories
            headerVisible: false

            TableViewColumn { }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                propagateComposedEvents: true

                onClicked: {
                    var index = tableView.rowAt(mouse.x, mouse.y)
                    if (index !== -1) {
                        tableView.currentRow = index
                        tableView.selection.clear()
                        tableView.selection.select(index)
                        itemMenu.popup()
                    }
                }
            }

            Keys.onPressed: {
                if (event.key === Qt.Key_F2 && renameAction.enabled) {
                    renameAction.trigger()
                }
            }

            onPressAndHold: itemMenu.popup()
            onDoubleClicked: showEntriesAction.trigger()
        }

        Row {
            Layout.fillWidth: true
            Layout.fillHeight: false
            spacing: 10

            Button {
                anchors.verticalCenter: parent.verticalCenter
                action: renameAction
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                action: showEntriesAction
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                action: showStatsAction
            }
        }
    }
}
