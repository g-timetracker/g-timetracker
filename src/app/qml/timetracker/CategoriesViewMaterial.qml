import QtQuick 2.0
import QtQuick.Controls.Private 1.0 as QQC1P
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQml.Models 2.2
import TimeLog 1.0

Item {
    property string title: qsTranslate("main window", "Categories")

    QtObject {
        id: d

        property bool isRemoveEnabled: !!treeView.currentItem
                                       && !treeView.currentItem.hasItems
        property bool isEditEnabled: !!treeView.currentItem
        property bool isShowEntriesEnabled: !!treeView.currentItem
        property bool isShowStatsEnabled: !!treeView.currentItem

        function create() {
            newDialog.setData()
            TimeTracker.showDialogRequested(newDialog)
        }

        function remove() {
            if (Settings.isConfirmationsEnabled) {
                removeConfirmationDialog.open()
            } else {
                categoryModel.removeItem(treeView.currentItem.fullName)
            }
        }

        function edit() {
            editDialog.setData(treeView.currentItem)
            TimeTracker.showDialogRequested(editDialog)
        }

        function showEntries() {
            TimeTracker.showSearchRequested(treeView.currentItem.fullName)
        }

        function showStats() {
            TimeTracker.showStatsRequested(treeView.currentItem.fullName)
        }
    }

    TimeLogCategoryTreeModel {
        id: categoryModel

        timeTracker: TimeTracker
    }

    QQC1P.TreeModelAdaptor {
        id: adaptorModel

        model: categoryModel
    }

    CategoryEditDialog {
        id: editDialog
    }

    CategoryNewDialog {
        id: newDialog

        onDataAccepted: categoryModel.addItem(newData)
    }

    RemoveConfirmationDialog {
        id: removeConfirmationDialog

        text: qsTranslate("categories view", "Remove this category?")

        onAccepted: categoryModel.removeItem(treeView.currentItem.fullName)
        onClosed: treeView.currentIndex = -1
    }

    Menu {
        id: itemMenu

        function popup() {
            x = mouseArea.mouseX
            y = mouseArea.mouseY
            open()
        }

        onClosed: {
            if (!removeConfirmationDialog.visible) {
                treeView.currentIndex = -1
            }
        }

        MenuItem {
            text: qsTranslate("categories view", "Edit")
            enabled: d.isEditEnabled
            onTriggered: d.edit()
        }
        MenuItem {
            text: qsTranslate("categories view", "Remove")
            enabled: d.isRemoveEnabled
            onTriggered: d.remove()
        }
        MenuItem {
            text: qsTranslate("categories view", "Show entries")
            enabled: d.isShowEntriesEnabled
            onTriggered: d.showEntries()
        }
        MenuItem {
            text: qsTranslate("categories view", "Show statistics")
            enabled: d.isShowStatsEnabled
            onTriggered: d.showStats()
        }
    }

    ListView {
        id: treeView

        anchors.fill: parent
        model: adaptorModel
        currentIndex: -1
        delegate: CategoryDelegate {
            width: treeView.width
            name: model.name
            fullName: model.fullName
            categoryData: model.data
            hasChildren: model._q_TreeView_HasChildren
            hasItems: model.hasItems
            isExpanded: model._q_TreeView_ItemExpanded
            depth: model._q_TreeView_ItemDepth
            isCurrent: ListView.isCurrentItem
            isLast: model.index === treeView.count - 1

            onClicked: {
                if (itemMenu.visible) {
                    return
                }

                treeView.currentIndex = model.index
                bottomSheet.open()
            }
            onExpand: adaptorModel.expand(adaptorModel.mapRowToModelIndex(model.index))
            onCollapse: adaptorModel.collapse(adaptorModel.mapRowToModelIndex(model.index))
        }

        ScrollBar.vertical: ScrollBar { }

        MouseArea {
            id: mouseArea

            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            propagateComposedEvents: true

            onClicked: {
                var index = treeView.indexAt(mouse.x + treeView.contentX,
                                             mouse.y + treeView.contentY)
                if (index > -1) {
                    treeView.currentIndex = index
                    itemMenu.popup()
                }
            }
        }
    }

    FloatingActionButton {
        iconSource: "images/ic_add_white_24dp.png"

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: height / 2
        onClicked: d.create()
    }

    Drawer {
        id: bottomSheet

        width: parent.width
        implicitHeight: bottomSheetItems.implicitHeight + 16
        edge: Qt.BottomEdge
        dragMargin: 0

        onClosed: {
            if (!removeConfirmationDialog.visible) {
                treeView.currentIndex = -1
            }
        }

        Column {
            id: bottomSheetItems

            width: parent.width
            y: 8

            ItemDelegateMaterial {
                width: parent.width
                text: qsTranslate("categories view", "Edit")
                iconItem.source: "images/ic_mode_edit_white_24dp.png"
                enabled: d.isEditEnabled
                onClicked: {
                    d.edit()
                    bottomSheet.close()
                }
            }
            ItemDelegateMaterial {
                width: parent.width
                text: qsTranslate("categories view", "Remove")
                iconItem.source: "images/ic_delete_white_24dp.png"
                enabled: d.isRemoveEnabled
                onClicked: {
                    d.remove()
                    bottomSheet.close()
                }
            }
            ItemDelegateMaterial {
                width: parent.width
                text: qsTranslate("categories view", "Show entries")
                iconItem.source: "images/ic_search_white_24dp.png"
                enabled: d.isShowEntriesEnabled
                onClicked: {
                    d.showEntries()
                    bottomSheet.close()
                }
            }
            ItemDelegateMaterial {
                width: parent.width
                text: qsTranslate("categories view", "Show statistics")
                iconItem.source: "images/ic_show_chart_white_24dp.png"
                enabled: d.isShowStatsEnabled
                onClicked: {
                    d.showStats()
                    bottomSheet.close()
                }
            }
        }
    }
}
