import QtQuick 2.0
import QtQuick.Controls 1.4 as QQC1
import Qt.labs.controls 1.0
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQml.Models 2.2
import TimeLog 1.0

Item {
    property string title: "Categories"

    QtObject {
        id: d

        property bool isRemoveEnabled: treeView.isCurrentIndexValid
                                       && !categoryModel.data(treeView.selection.currentIndex,
                                                              TimeLogCategoryTreeModel.HasItemsRole)
        property bool isEditEnabled: treeView.isCurrentIndexValid
        property bool isShowEntriesEnabled: treeView.isCurrentIndexValid
        property bool isShowStatsEnabled: treeView.isCurrentIndexValid

        function create() {
            newDialog.openDialog()
        }

        function remove() {
            if (Settings.isConfirmationsEnabled) {
                removeConfirmationDialog.open()
            } else {
                categoryModel.removeItem(treeView.selection.currentIndex)
            }
        }

        function edit() {
            editDialog.openDialog(categoryModel.data(treeView.selection.currentIndex,
                                                     TimeLogCategoryTreeModel.FullNameRole),
                                  categoryModel.data(treeView.selection.currentIndex,
                                                     TimeLogCategoryTreeModel.DataRole))
        }

        function showEntries() {
            TimeTracker.showSearchRequested(categoryModel.data(treeView.selection.currentIndex,
                                                               TimeLogCategoryTreeModel.FullNameRole))
        }

        function showStats() {
            TimeTracker.showStatsRequested(categoryModel.data(treeView.selection.currentIndex,
                                                              TimeLogCategoryTreeModel.FullNameRole))
        }
    }

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

        function popup() {
            x = mouseArea.mouseX
            y = mouseArea.mouseY
            open()
        }

        MenuItem {
            text: "Remove"
            enabled: d.isRemoveEnabled
            onTriggered: d.remove()
        }
        MenuItem {
            text: "Edit"
            enabled: d.isEditEnabled
            onTriggered: d.edit()
        }
        MenuItem {
            text: "Show entries"
            enabled: d.isShowEntriesEnabled
            onTriggered: d.showEntries()
        }
        MenuItem {
            text: "Show statistics"
            enabled: d.isShowStatsEnabled
            onTriggered: d.showStats()
        }
    }

    ColumnLayout {
        anchors.fill: parent

        QQC1.TreeView {
            id: treeView

            property bool isCurrentIndexValid: selection.currentIndex.valid

            Layout.fillWidth: true
            Layout.fillHeight: true
            model: categoryModel
            headerVisible: true
            selectionMode: QQC1.SelectionMode.SingleSelection
            selection: ItemSelectionModel {
                model: categoryModel
            }

            QQC1.TableViewColumn {
                role: "name"
            }

            QQC1.TableViewColumn {
                role: "fullName"
            }

            QQC1.TableViewColumn {
                role: "comment"
            }

            QQC1.TableViewColumn {
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
                if (event.key === Qt.Key_F2 && d.isEditEnabled) {
                    d.edit()
                }
            }

            onPressAndHold: itemMenu.popup()
            onDoubleClicked: d.showEntries()
        }

        Row {
            Layout.fillWidth: true
            Layout.fillHeight: false
            spacing: 10

            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                text: "Create"
                onClicked: d.create()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: d.isRemoveEnabled
                text: "Remove"
                onClicked: d.remove()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: d.isEditEnabled
                text: "Edit"
                onClicked: d.edit()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: d.isShowEntriesEnabled
                text: "Show entries"
                onClicked: d.showEntries()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: d.isShowStatsEnabled
                text: "Show statistics"
                onClicked: d.showStats()
            }
        }
    }
}
