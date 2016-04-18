import QtQuick 2.4
import QtQuick.Layouts 1.1
import Qt.labs.controls 1.0
import QtQml.Models 2.2
import TimeLog 1.0
import "Util.js" as Util

Item {
    id: timeLogView

    property alias model: delegateModel.model
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

    signal insert(var modelIndex, var newData)
    signal append(var newData)
    signal remove(var modelIndex)

    function itemEdit() {
        editDialog.setData(listView.itemUnderCursor())
        TimeTracker.showDialogRequested(editDialog)
    }

    function itemInsertBefore() {
        var index = listView.indexUnderCursor()
        var item = listView.itemUnderCursor()
        d.insert(index, item.precedingStart, item.startTime)
    }

    function itemInsertAfter() {
        var index = listView.indexUnderCursor()
        var item = listView.itemUnderCursor()
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
            d.deleteItemUnderCursor()
        }
    }

    function pointedItem() {
        return listView.itemUnderCursor()
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

        function deleteItemUnderCursor() {
            timeLogView.remove(delegateModel.modelIndex(listView.indexUnderCursor()))
        }
    }

    Menu {
        id: itemMenu

        function popup() {
            x = mouseArea.mouseX
            y = mouseArea.mouseY
            open()
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
            isLastItem: model.index === delegateModel.count
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

        onAccepted: d.deleteItemUnderCursor()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ListView {
            id: listView

            function itemUnderCursor() {
                return itemAt(mouseArea.mouseX + contentX, mouseArea.mouseY + contentY)
            }

            function indexUnderCursor() {
                return indexAt(mouseArea.mouseX + contentX, mouseArea.mouseY + contentY)
            }

            Layout.fillHeight: true
            Layout.fillWidth: true
            verticalLayoutDirection: timeLogView.reverse ? ListView.BottomToTop : ListView.TopToBottom
            clip: true
            model: delegateModel

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onDoubleClicked: {
                    if (listView.itemAt(mouse.x + listView.contentX,
                                        mouse.y + listView.contentY)) {
                        timeLogView.itemEdit()
                    }
                }

                onPressAndHold:{
                    if (mouse.button === Qt.LeftButton && listView.itemAt(mouse.x + listView.contentX,
                                                                          mouse.y + listView.contentY)) {
                        itemMenu.popup()
                    }
                }

                onClicked: {
                    if (mouse.button === Qt.RightButton && listView.itemAt(mouse.x + listView.contentX,
                                                                           mouse.y + listView.contentY)) {
                        itemMenu.popup()
                    }
                }
            }
        }
        Item {
            Layout.preferredHeight: (!timeLogView.reverse || listView.contentHeight > parent.height ? 0 : parent.height - listView.contentHeight)
        }
    }
}
