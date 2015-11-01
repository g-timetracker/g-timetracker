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

    signal insert(var modelIndex, var newData)
    signal append(var newData)
    signal remove(var modelIndex)

    function appendDialog() {
        var timeAfter = delegateModel.items.count ? delegateModel.items.get(0).model.startTime
                                                  : new Date(0)
        d.insert(-1, timeAfter, new Date())
    }

    QtObject {
        id: d

        function insert(indexBefore, timeAfter, timeBefore) {
            if (Util.calcDuration(timeAfter, timeBefore) > 1) {
                newDialog.openDialog(indexBefore, timeAfter, timeBefore)
            } else {
                TimeLog.error("Cannot insert between %1 and %2".arg(timeAfter).arg(timeBefore))
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
            durationTime: model.durationTime
            comment: model.comment
            precedingStart: model.precedingStart
            succeedingStart: model.succeedingStart
        }
    }

    TimeLogEditDialog {
        id: editDialog

        onError: TimeLog.error(errorText)
    }

    TimeLogNewDialog {
        id: newDialog

        onDataAccepted: {
            if (newDialog.indexBefore === -1) {
                timeLogView.append(newData)
            } else {
                timeLogView.insert(delegateModel.modelIndex(newDialog.indexBefore), newData)
            }
        }

        onError: TimeLog.error(errorText)
    }

    Action {
        id: editAction

        text: "Edit"
        tooltip: "Edit item"

        onTriggered: editDialog.openDialog(listView.itemUnderCursor())
    }

    Action {
        id: insertBeforeAction

        text: "Insert before"
        tooltip: "Insert item before this item"

        onTriggered: {
            var index = listView.indexUnderCursor()
            var item = listView.itemUnderCursor()
            d.insert(index, item.precedingStart, item.startTime)
        }
    }

    Action {
        id: insertAfterAction

        text: "Insert after"
        tooltip: "Insert item after this item"

        onTriggered: {
            var index = listView.indexUnderCursor()
            var item = listView.itemUnderCursor()
            d.insert(index - 1, item.startTime, item.succeedingStart)
        }
    }

    Action {
        id: removeAction

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
        informativeText: "This item can not be restored"
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes | StandardButton.No

        onYes: d.deleteItemUnderCursor()
    }

    Menu {
        id: itemMenu

        MenuItem {
            action: editAction
        }
        MenuItem {
            action: insertBeforeAction
        }
        MenuItem {
            action: insertAfterAction
        }
        MenuItem {
            action: removeAction
        }
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
            verticalLayoutDirection: ListView.BottomToTop
            clip: true
            model: delegateModel

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onDoubleClicked: {
                    if (listView.itemAt(mouse.x + listView.contentX,
                                        mouse.y + listView.contentY)) {
                        editAction.trigger()
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
            Layout.preferredHeight: (listView.contentHeight > parent.height ? 0 : parent.height - listView.contentHeight)
        }
    }
}
