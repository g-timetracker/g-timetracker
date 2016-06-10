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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import QtQml.Models 2.2
import TimeLog 1.0
import "Util.js" as Util

Item {
    id: timeLogView

    property alias model: delegateModel.model
    property alias currentItem: listView.currentItem
    property bool reverseModel: false
    property bool reverseView: false
    property bool lazyDialogs: true

    property alias menuModel: itemMenu.contentData
    property MenuItem editMenuItem: MenuItemMaterial {
        text: qsTr("Edit")
        iconItem.source: "images/ic_mode_edit_white_24dp.png"
        onTriggered: timeLogView.itemEdit()
    }
    property MenuItem addAboveMenuItem: MenuItemMaterial {
        text: d.isSwapBeforeAndAfter ? qsTr("Add after") : qsTr("Add before")
        iconItem.source: "images/ic_subdirectory_arrow_right_white_24dp.png"
        iconItem.transform: Scale {
            yScale: -1
            origin.y: 12
        }
        onTriggered: timeLogView.itemAddAbove()
    }
    property MenuItem addBelowMenuItem: MenuItemMaterial {
        text: d.isSwapBeforeAndAfter ? qsTr("Add before") : qsTr("Add after")
        iconItem.source: "images/ic_subdirectory_arrow_right_white_24dp.png"
        onTriggered: timeLogView.itemAddBelow()
    }
    property MenuItem removeMenuItem: MenuItemMaterial {
        text: qsTr("Delete")
        iconItem.source: "images/ic_delete_white_24dp.png"
        onTriggered: timeLogView.itemRemove()
    }

    property alias bottomSheetModel: bottomSheetItems.children
    property ItemDelegate editBottomSheetItem: ItemDelegateMaterial {
        width: bottomSheetItems.width
        text: qsTr("Edit")
        iconItem.source: "images/ic_mode_edit_white_24dp.png"
        onClicked: {
            timeLogView.itemEdit()
            timeLogView.closeBottomSheet()
        }
    }
    property ItemDelegate addAboveBottomSheetItem: ItemDelegateMaterial {
        width: bottomSheetItems.width
        text: d.isSwapBeforeAndAfter ? qsTr("Add after") : qsTr("Add before")
        iconItem.source: "images/ic_subdirectory_arrow_right_white_24dp.png"
        iconItem.transform: Scale {
            yScale: -1
            origin.y: 12
        }
        onClicked: {
            timeLogView.itemAddAbove()
            timeLogView.closeBottomSheet()
        }
    }
    property ItemDelegate addBelowBottomSheetItem: ItemDelegateMaterial {
        width: bottomSheetItems.width
        text: d.isSwapBeforeAndAfter ? qsTr("Add before") : qsTr("Add after")
        iconItem.source: "images/ic_subdirectory_arrow_right_white_24dp.png"
        onClicked: {
            timeLogView.itemAddBelow()
            timeLogView.closeBottomSheet()
        }
    }
    property ItemDelegate removeBottomSheetItem: ItemDelegateMaterial {
        id: control
        width: bottomSheetItems.width
        text: qsTr("Delete")
        iconItem.source: "images/ic_delete_white_24dp.png"
        onClicked: {
            timeLogView.itemRemove()
            timeLogView.closeBottomSheet()
        }
    }

    signal insert(var modelIndex, var newData)
    signal append(var newData)
    signal remove(var modelIndex)

    function itemEdit() {
        editDialog.get().setData(listView.currentItem)
        TimeTracker.showDialogRequested(editDialog.get())
    }

    function itemAddAbove() {
        if (d.isSwapBeforeAndAfter) {
            d.addAfter()
        } else {
            d.addBefore()
        }
    }

    function itemAddBelow() {
        if (d.isSwapBeforeAndAfter) {
            d.addBefore()
        } else {
            d.addAfter()
        }
    }

    function itemAppend() {
        var timeAfter = delegateModel.items.count ? delegateModel.items.get(0).model.startTime
                                                  : new Date(0)
        d.add(timeLogView.reverseModel ? -1 : delegateModel.items.count - 1, timeAfter, new Date())
    }

    function itemRemove() {
        if (AppSettings.isConfirmationsEnabled) {
            removeConfirmationDialog.get().open()
        } else {
            d.deleteCurrentItem()
        }
    }

    function closeBottomSheet() {
        bottomSheet.close()
    }

    QtObject {
        id: d

        property bool isSwapBeforeAndAfter: timeLogView.reverseModel !== timeLogView.reverseView

        function addBefore() {
            var index = listView.currentIndex
            var item = listView.currentItem
            d.add(index, item.precedingStart, item.startTime)
        }

        function addAfter() {
            var index = listView.currentIndex
            var item = listView.currentItem
            d.add(timeLogView.reverseModel ? index - 1 : index + 1, item.startTime, item.succeedingStart)
        }

        function add(indexBefore, timeAfter, timeBefore) {
            if (Util.calcDuration(timeAfter, timeBefore) > 1) {
                newDialog.get().setData(indexBefore, timeAfter, timeBefore)
                TimeTracker.showDialogRequested(newDialog.get())
            } else {
                TimeTracker.error(qsTr("Cannot add entry between %1 and %2").arg(timeAfter).arg(timeBefore))
            }
        }

        function deleteCurrentItem() {
            timeLogView.remove(delegateModel.modelIndex(listView.currentIndex))
        }
    }

    Menu {
        id: itemMenu

        function popup() {
            x = mouseArea.mouseX
            y = mouseArea.mouseY
            open()
        }

        onClosed: {
            if (!removeConfirmationDialog.get().visible) {
                listView.currentIndex = -1
            }
        }

         contentData: [
             editMenuItem,
             addAboveMenuItem,
             addBelowMenuItem,
             removeMenuItem
        ]
    }

    DelegateModel {
        id: delegateModel

        delegate: TimeLogDelegate {
            width: listView.width
            category: model.category
            startTime: model.startTime
            durationTime: (model.durationTime === -1 ? Util.calcDuration(startTime, new Date())
                                                     : model.durationTime)
            comment: model.comment
            precedingStart: model.precedingStart
            succeedingStart: model.succeedingStart
            isCurrent: ListView.isCurrentItem
            isLast: model.index === (timeLogView.reverseView ? 0 : listView.count - 1)
        }
    }

    LazyLoader {
        id: editDialog

        isLazy: timeLogView.lazyDialogs
        sourceComponent: TimeLogEditDialog {
            onError: TimeTracker.error(errorText)
        }
    }

    LazyLoader {
        id: newDialog

        isLazy: timeLogView.lazyDialogs
        sourceComponent: TimeLogNewDialog {
            id: dialog

            onDataAccepted: {
                if (dialog.indexBefore === (timeLogView.reverseModel ? -1 : delegateModel.items.count - 1)) {
                    timeLogView.append(newData)
                } else {
                    timeLogView.insert(delegateModel.modelIndex(dialog.indexBefore), newData)
                }
            }

            onError: TimeTracker.error(errorText)
        }
    }

    LazyLoader {
        id: removeConfirmationDialog

        isLazy: timeLogView.lazyDialogs
        sourceComponent: RemoveConfirmationDialog {
            text: qsTr("Delete this entry?")

            onAccepted: d.deleteCurrentItem()
            onClosed: listView.currentIndex = -1
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ListView {
            id: listView

            Layout.fillHeight: true
            Layout.fillWidth: true
            verticalLayoutDirection: timeLogView.reverseView ? ListView.BottomToTop : ListView.TopToBottom
            clip: true
            model: delegateModel
            currentIndex: -1

            ScrollBar.vertical: ScrollBar { }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                Timer {
                    id: singleClickTimer

                    interval: 100

                    onTriggered: bottomSheet.open()
                }

                onDoubleClicked: {
                    singleClickTimer.stop()
                    timeLogView.itemEdit()
                    listView.currentIndex = -1
                }

                onPressAndHold:{
                    var index = listView.indexAt(mouse.x + listView.contentX,
                                                 mouse.y + listView.contentY)
                    if (index > -1 && mouse.button === Qt.LeftButton) {
                        listView.currentIndex = index
                        itemMenu.popup()
                    }
                }

                onClicked: {
                    if (itemMenu.visible) {
                        return
                    }

                    var index = listView.indexAt(mouse.x + listView.contentX,
                                                 mouse.y + listView.contentY)
                    if (index > -1) {
                        listView.currentIndex = index
                        if (mouse.button === Qt.RightButton) {
                            itemMenu.popup()
                        } else if (PlatformMaterial.isDesktop) {  // Double-clicks are only for desktop
                            singleClickTimer.start()
                        } else {
                            bottomSheet.open()
                        }
                    }
                }
            }
        }
        Item {
            Layout.preferredHeight: (!timeLogView.reverseView || listView.contentHeight > parent.height ? 0 : parent.height - listView.contentHeight)
        }
    }

    Drawer {
        id: bottomSheet

        width: parent.width
        implicitHeight: bottomSheetItems.implicitHeight + 16
        edge: Qt.BottomEdge
        dragMargin: 0

        onClosed: {
            if (!removeConfirmationDialog.get().visible) {
                listView.currentIndex = -1
            }
        }

        Column {
            id: bottomSheetItems

            width: parent.width
            y: 8

            children: [
                editBottomSheetItem,
                addAboveBottomSheetItem,
                addBelowBottomSheetItem,
                removeBottomSheetItem
            ]
        }
    }
}
