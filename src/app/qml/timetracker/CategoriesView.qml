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

    CategoryEditDialog {
        id: editDialog

        function openDialog(name, data) {
            setData(name, data)
            TimeTracker.showDialogRequested(editDialog)
        }

        onDataAccepted: categoryModel.editItem(treeView.selection.currentIndex, newData)
    }

    CategoryNewDialog {
        id: newDialog

        function openDialog() {
            setData()
            TimeTracker.showDialogRequested(newDialog)
        }

        onDataAccepted: categoryModel.addItem(newData)
    }

    Action {
        id: newAction

        text: "Create"
        tooltip: "Create category"

        onTriggered: newDialog.openDialog()
    }

    Action {
        id: removeAction

        text: "Remove"
        tooltip: "Remove category"
        enabled: treeView.isCurrentIndexValid
                 && !categoryModel.data(treeView.selection.currentIndex,
                                        TimeLogCategoryTreeModel.HasItemsRole)

        onTriggered: {
            if (Settings.isConfirmationsEnabled) {
                removeConfirmationDialog.open()
            } else {
                categoryModel.removeItem(treeView.selection.currentIndex)
            }
        }
    }

    Action {
        id: editAction

        text: "Edit"
        tooltip: "Edit category"
        enabled: treeView.isCurrentIndexValid

        onTriggered: editDialog.openDialog(categoryModel.data(treeView.selection.currentIndex,
                                                              TimeLogCategoryTreeModel.FullNameRole),
                                           categoryModel.data(treeView.selection.currentIndex,
                                                              TimeLogCategoryTreeModel.DataRole))
    }

    Action {
        id: showEntriesAction

        text: "Show entries"
        tooltip: "Show entries for this category"
        enabled: treeView.isCurrentIndexValid

        onTriggered: TimeTracker.showSearchRequested(categoryModel.data(treeView.selection.currentIndex,
                                                                        TimeLogCategoryTreeModel.FullNameRole))
    }

    Action {
        id: showStatsAction

        text: "Show statistics"
        tooltip: "Show statistics for this category"
        enabled: treeView.isCurrentIndexValid

        onTriggered: TimeTracker.showStatsRequested(categoryModel.data(treeView.selection.currentIndex,
                                                                       TimeLogCategoryTreeModel.FullNameRole))
    }

    MessageDialog {
        id: removeConfirmationDialog

        title: "Remove confirmation"
        text: "Are you sure want to delete this category?"
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes | StandardButton.No

        onYes: categoryModel.removeItem(treeView.selection.currentIndex)
    }

    Menu {
        id: itemMenu

        MenuItem {
            action: removeAction
        }
        MenuItem {
            action: editAction
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

            TableViewColumn {
                role: "comment"
            }

            TableViewColumn {
                role: "hasItems"
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
                if (event.key === Qt.Key_F2 && editAction.enabled) {
                    editAction.trigger()
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
                text: newAction.text
                onClicked: newAction.trigger()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: removeAction.enabled
                text: removeAction.text
                onClicked: removeAction.trigger()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: editAction.enabled
                text: editAction.text
                onClicked: editAction.trigger()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: showEntriesAction.enabled
                text: showEntriesAction.text
                onClicked: showEntriesAction.trigger()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: showStatsAction.enabled
                text: showStatsAction.text
                onClicked: showStatsAction.trigger()
            }
        }
    }
}
