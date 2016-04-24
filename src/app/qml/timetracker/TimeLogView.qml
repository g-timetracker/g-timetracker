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
    property bool reverse: false

    property alias menuModel: itemMenu.contentData
    property MenuItem editMenuItem: MenuItem {
        text: qsTr("Edit")
        onTriggered: timeLogView.itemEdit()
    }
    property MenuItem insertBeforeMenuItem: MenuItem {
        text: qsTr("Insert before")
        onTriggered: timeLogView.itemInsertBefore()
    }
    property MenuItem insertAfterMenuItem: MenuItem {
        text: qsTr("Insert after")
        onTriggered: timeLogView.itemInsertAfter()
    }
    property MenuItem removeMenuItem: MenuItem {
        text: qsTr("Remove")
        onTriggered: timeLogView.itemRemove()
    }

    property alias bottomSheetModel: bottomSheetItems.children
    property ItemDelegate editBottomSheetItem: ItemDelegate {
        width: bottomSheetItems.width
        text: qsTr("Edit")
        onClicked: {
            timeLogView.itemEdit()
            timeLogView.closeBottomSheet()
        }
    }
    property ItemDelegate insertBeforeBottomSheetItem: ItemDelegate {
        width: bottomSheetItems.width
        text: qsTr("Insert before")
        onClicked: {
            timeLogView.itemInsertBefore()
            timeLogView.closeBottomSheet()
        }
    }
    property ItemDelegate insertAfterBottomSheetItem: ItemDelegate {
        width: bottomSheetItems.width
        text: qsTr("Insert after")
        onClicked: {
            timeLogView.itemInsertAfter()
            timeLogView.closeBottomSheet()
        }
    }
    property ItemDelegate removeBottomSheetItem: ItemDelegate {
        width: bottomSheetItems.width
        text: qsTr("Remove")
        onClicked: {
            timeLogView.itemRemove()
            timeLogView.closeBottomSheet()
        }
    }

    signal insert(var modelIndex, var newData)
    signal append(var newData)
    signal remove(var modelIndex)

    function itemEdit() {
        editDialog.setData(listView.currentItem)
        TimeTracker.showDialogRequested(editDialog)
    }

    function itemInsertBefore() {
        var index = listView.currentIndex
        var item = listView.currentItem
        d.insert(index, item.precedingStart, item.startTime)
    }

    function itemInsertAfter() {
        var index = listView.currentIndex
        var item = listView.currentItem
        d.insert(timeLogView.reverse ? index - 1 : index + 1, item.startTime, item.succeedingStart)
    }

    function itemAppend() {
        var timeAfter = delegateModel.items.count ? delegateModel.items.get(0).model.startTime
                                                  : new Date(0)
        d.insert(timeLogView.reverse ? -1 : delegateModel.items.count - 1, timeAfter, new Date())
    }

    function itemRemove() {
        if (Settings.isConfirmationsEnabled) {
            removeConfirmationDialog.open()
        } else {
            d.deleteCurrentItem()
        }
    }

    function closeBottomSheet() {
        bottomSheet.close()
    }

    QtObject {
        id: d

        function insert(indexBefore, timeAfter, timeBefore) {
            if (Util.calcDuration(timeAfter, timeBefore) > 1) {
                newDialog.setData(indexBefore, timeAfter, timeBefore)
                TimeTracker.showDialogRequested(newDialog)
            } else {
                TimeTracker.error(qsTr("Cannot insert between %1 and %2").arg(timeAfter).arg(timeBefore))
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
            if (!removeConfirmationDialog.visible) {
                listView.currentIndex = -1
            }
        }

         contentData: [
             editMenuItem,
             insertBeforeMenuItem,
             insertAfterMenuItem,
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
            isLast: model.index === (timeLogView.reverse ? 0 : listView.count - 1)
        }
    }

    TimeLogEditDialog {
        id: editDialog

        onError: TimeTracker.error(errorText)
    }

    TimeLogNewDialog {
        id: newDialog

        onDataAccepted: {
            if (newDialog.indexBefore === (timeLogView.reverse ? -1 : delegateModel.items.count - 1)) {
                timeLogView.append(newData)
            } else {
                timeLogView.insert(delegateModel.modelIndex(newDialog.indexBefore), newData)
            }
        }

        onError: TimeTracker.error(errorText)
    }

    RemoveConfirmationDialog {
        id: removeConfirmationDialog

        text: qsTr("Delete this entry?")

        onAccepted: d.deleteCurrentItem()
        onClosed: listView.currentIndex = -1
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ListView {
            id: listView

            Layout.fillHeight: true
            Layout.fillWidth: true
            verticalLayoutDirection: timeLogView.reverse ? ListView.BottomToTop : ListView.TopToBottom
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
                        } else if (Qt.platform.os !== "android") {  // Double-clicks are only for desktop
                            singleClickTimer.start()
                        } else {
                            bottomSheet.open()
                        }
                    }
                }
            }
        }
        Item {
            Layout.preferredHeight: (!timeLogView.reverse || listView.contentHeight > parent.height ? 0 : parent.height - listView.contentHeight)
        }
    }

    Drawer {
        id: bottomSheet

        width: parent.width
        implicitHeight: bottomSheetItems.implicitHeight + 16
        edge: Qt.BottomEdge
        dragMargin: 0

        onClosed: {
            if (!removeConfirmationDialog.visible) {
                listView.currentIndex = -1
            }
        }

        Column {
            id: bottomSheetItems

            width: parent.width
            y: 8

            children: [
                editBottomSheetItem,
                insertBeforeBottomSheetItem,
                insertAfterBottomSheetItem,
                removeBottomSheetItem
            ]
        }
    }
}
