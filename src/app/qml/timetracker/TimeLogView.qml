import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQml.Models 2.2
import TimeLog 1.0
import "Util.js" as Util

Item {
    id: timeLogView

    property alias model: delegateModel.model
    property bool reverse: false

    property alias menu: d.itemMenu
    readonly property alias editAction: itemEditAction
    readonly property alias insertBeforeAction: itemInsertBeforeAction
    readonly property alias insertAfterAction: itemInsertAfterAction
    readonly property alias appendAction: itemAppendAction
    readonly property alias removeAction: itemRemoveAction

    signal insert(var modelIndex, var newData)
    signal append(var newData)
    signal remove(var modelIndex)

    function pointedItem() {
        return listView.itemUnderCursor()
    }

    QtObject {
        id: d

        property Menu itemMenu: Menu {
            MenuItem {
                action: itemEditAction
            }
            MenuItem {
                action: itemInsertBeforeAction
            }
            MenuItem {
                action: itemInsertAfterAction
            }
            MenuItem {
                action: itemRemoveAction
            }
        }

        function insert(indexBefore, timeAfter, timeBefore) {
            if (Util.calcDuration(timeAfter, timeBefore) > 1) {
                newDialog.setData(indexBefore, timeAfter, timeBefore)
                MainWindow.showDialog(newDialog)
            } else {
                TimeTracker.error("Cannot insert between %1 and %2".arg(timeAfter).arg(timeBefore))
            }
        }

        function deleteItemUnderCursor() {
            timeLogView.remove(delegateModel.modelIndex(listView.indexUnderCursor()))
        }
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

    Action {
        id: itemEditAction

        text: "Edit"
        tooltip: "Edit item"

        onTriggered: {
            editDialog.setData(listView.itemUnderCursor())
            MainWindow.showDialog(editDialog)
        }
    }

    Action {
        id: itemInsertBeforeAction

        text: "Insert before"
        tooltip: "Insert item before this item"

        onTriggered: {
            var index = listView.indexUnderCursor()
            var item = listView.itemUnderCursor()
            d.insert(index, item.precedingStart, item.startTime)
        }
    }

    Action {
        id: itemInsertAfterAction

        text: "Insert after"
        tooltip: "Insert item after this item"

        onTriggered: {
            var index = listView.indexUnderCursor()
            var item = listView.itemUnderCursor()
            d.insert(timeLogView.reverse ? index - 1 : index + 1, item.startTime, item.succeedingStart)
        }
    }

    Action {
        id: itemAppendAction

        text: "Add item"
        tooltip: "Adds item into model"

        onTriggered: {
            var timeAfter = delegateModel.items.count ? delegateModel.items.get(0).model.startTime
                                                      : new Date(0)
            d.insert(timeLogView.reverse ? -1 : delegateModel.items.count - 1, timeAfter, new Date())
        }
    }

    Action {
        id: itemRemoveAction

        text: "Remove"
        tooltip: "Remove item"

        onTriggered: {
            if (Settings.isConfirmationsEnabled) {
                removeConfirmationDialog.open()
            } else {
                d.deleteItemUnderCursor()
            }
        }
    }

    MessageDialog {
        id: removeConfirmationDialog

        title: "Remove confirmation"
        text: "Are you sure want to delete this item?"
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes | StandardButton.No

        onYes: d.deleteItemUnderCursor()
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
                        itemEditAction.trigger()
                    }
                }

                onPressAndHold:{
                    if (mouse.button === Qt.LeftButton && listView.itemAt(mouse.x + listView.contentX,
                                                                          mouse.y + listView.contentY)) {
                        d.itemMenu.popup()
                    }
                }

                onClicked: {
                    if (mouse.button === Qt.RightButton && listView.itemAt(mouse.x + listView.contentX,
                                                                           mouse.y + listView.contentY)) {
                        d.itemMenu.popup()
                    }
                }
            }
        }
        Item {
            Layout.preferredHeight: (!timeLogView.reverse || listView.contentHeight > parent.height ? 0 : parent.height - listView.contentHeight)
        }
    }
}
