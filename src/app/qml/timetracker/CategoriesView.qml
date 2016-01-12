import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQml.Models 2.2
import TimeLog 1.0

Item {
    property string title: "Categories"

    TimeLogCategoryTreeModel {
        id: categoryModel

        timeTracker: TimeTracker
    }

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
        enabled: treeView.isCurrentIndexValid

        onTriggered: renameDialog.openDialog(categoryModel.data(treeView.selection.currentIndex,
                                                                TimeLogCategoryTreeModel.FullNameRole))
    }

    Action {
        id: showEntriesAction

        text: "Show entries"
        tooltip: "Show entries for this category"
        enabled: treeView.isCurrentIndexValid

        onTriggered: MainWindow.showSearch(categoryModel.data(treeView.selection.currentIndex,
                                                                         TimeLogCategoryTreeModel.FullNameRole))
    }

    Action {
        id: showStatsAction

        text: "Show statistics"
        tooltip: "Show statistics for this category"
        enabled: treeView.isCurrentIndexValid

        onTriggered: MainWindow.showStats(categoryModel.data(treeView.selection.currentIndex,
                                                                        TimeLogCategoryTreeModel.FullNameRole))
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

        TreeView {
            id: treeView

            property bool isCurrentIndexValid: selection.currentIndex.valid

            Layout.fillWidth: true
            Layout.fillHeight: true
            model: categoryModel
            headerVisible: true
            selectionMode: SelectionMode.SingleSelection
            selection: ItemSelectionModel {
                model: categoryModel
            }

            TableViewColumn {
                role: "name"
            }

            TableViewColumn {
                role: "fullName"
            }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                propagateComposedEvents: true

                onClicked: {
                    var index = treeView.indexAt(mouse.x, mouse.y)
                    if (index.valid) {
                        treeView.selection.setCurrentIndex(index, ItemSelectionModel.SelectCurrent)
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

            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                text: renameAction.text
                onClicked: renameAction.trigger()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                text: showEntriesAction.text
                onClicked: showEntriesAction.trigger()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                text: showStatsAction.text
                onClicked: showStatsAction.trigger()
            }
        }
    }
}
