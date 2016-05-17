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
import QtQuick.Controls.Private 1.0 as QQC1P
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQml.Models 2.2
import TimeLog 1.0

Page {
    id: view

    title: qsTranslate("main window", "Categories")

    header: MainToolBarMaterial {
        title: view.title
        isBottomItem: view.StackView.index === 0
    }

    QtObject {
        id: d

        property bool isRemoveEnabled: !!treeView.currentItem
                                       && !treeView.currentItem.hasItems
        property bool isEditEnabled: !!treeView.currentItem
        property bool isAddSubcategoryEnabled: !!treeView.currentItem
        property bool isShowEntriesEnabled: !!treeView.currentItem
        property bool isShowStatsEnabled: !!treeView.currentItem

        function create(parentName) {
            newDialog.setData(parentName ? parentName + " > " : "")
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

        text: qsTranslate("categories view", "Delete this category?")

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

        MenuItemMaterial {
            text: qsTranslate("categories view", "Edit")
            iconItem.source: "images/ic_mode_edit_white_24dp.png"
            enabled: d.isEditEnabled
            onTriggered: d.edit()
        }
        MenuItemMaterial {
            text: qsTranslate("categories view", "Add subcategory")
            iconItem.source: "images/ic_add_white_24dp.png"
            enabled: d.isAddSubcategoryEnabled
            onTriggered: d.create(treeView.currentItem.fullName)
        }
        MenuItemMaterial {
            text: qsTranslate("categories view", "Delete")
            iconItem.source: "images/ic_delete_white_24dp.png"
            enabled: d.isRemoveEnabled
            onTriggered: d.remove()
        }
        MenuItemMaterial {
            text: qsTranslate("categories view", "Show entries")
            iconItem.source: "images/ic_search_white_24dp.png"
            enabled: d.isShowEntriesEnabled
            onTriggered: d.showEntries()
        }
        MenuItemMaterial {
            text: qsTranslate("categories view", "Show statistics")
            iconItem.source: "images/ic_show_chart_white_24dp.png"
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
                text: qsTranslate("categories view", "Add subcategory")
                iconItem.source: "images/ic_add_white_24dp.png"
                enabled: d.isAddSubcategoryEnabled
                onClicked: {
                    d.create(treeView.currentItem.fullName)
                    bottomSheet.close()
                }
            }
            ItemDelegateMaterial {
                width: parent.width
                text: qsTranslate("categories view", "Delete")
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
