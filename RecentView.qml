import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQml.Models 2.2
import TimeLog 1.0
import "Util.js" as Util

Item {
    id: recentView

    property bool isConfirmationsEnabled: true

    ReverseProxyModel {
        id: timeLogModel

        sourceModel: TimeLogRecentModel { }
    }

    TimeLogEditDialog {
        id: editDialog
    }

    TimeLogNewDialog {
        id: newDialog

        onDataAccepted: {
            if (newDialog.indexBefore === -1) {
                timeLogModel.appendItem(newData)
            } else {
                timeLogModel.insertItem(delegateModel.modelIndex(newDialog.indexBefore), newData)
            }
        }
    }

    DelegateModel {
        id: delegateModel

        function timeAfterFromIndex(indexAfter) {
            if (indexAfter === -1) {
                return new Date(0)
            } else {
                return timeLogModel.timeLogData(delegateModel.modelIndex(indexAfter)).startTime
            }
        }

        function timeBeforeFromIndex(indexBefore) {
            if (indexBefore === -1) {
                return new Date()
            } else {
                return timeLogModel.timeLogData(delegateModel.modelIndex(indexBefore)).startTime
            }
        }

        function insert(indexBefore, timeAfter, timeBefore) {
            if (Util.calcDuration(timeAfter, timeBefore) > 1) {
                newDialog.openDialog(indexBefore, timeAfter, timeBefore)
            } else {
                TimeLog.error("Cannot insert between %1 and %2".arg(timeAfter).arg(timeBefore))
            }
        }

        model: timeLogModel
        delegate: TimeLogDelegate {
            id: delegateItem

            function updateData(category, startTime, comment) {
                if (model.category != category) {
                    model.category = category
                }
                if (model.startTime != startTime) {
                    model.startTime = startTime
                }
                if (model.comment != comment) {
                    model.comment = comment
                }
            }

            width: listView.width
            category: model.category
            startTime: model.startTime
            durationTime: model.durationTime
            comment: model.comment
        }
    }

    Action {
        id: editAction

        text: "Edit"
        tooltip: "Edit item"

        onTriggered: {
            var indexCurrent = listView.indexAt(mouseArea.mouseX + listView.contentX,
                                                mouseArea.mouseY + listView.contentY)
            var delegateItem = listView.itemAt(mouseArea.mouseX + listView.contentX,
                                               mouseArea.mouseY + listView.contentY)
            var indexAfter = (indexCurrent + 1 === listView.count) ? -1 : indexCurrent + 1
            var indexBefore = indexCurrent - 1
            editDialog.openDialog(delegateItem,
                                  delegateModel.timeAfterFromIndex(indexAfter),
                                  delegateModel.timeBeforeFromIndex(indexBefore))
        }
    }

    Action {
        id: insertBeforeAction

        text: "Insert before"
        tooltip: "Insert item before this item"

        onTriggered: {
            var indexBefore = listView.indexAt(mouseArea.mouseX + listView.contentX,
                                               mouseArea.mouseY + listView.contentY)
            var indexAfter = (indexBefore + 1 === listView.count) ? -1 : indexBefore + 1
            delegateModel.insert(indexBefore,
                                 delegateModel.timeAfterFromIndex(indexAfter),
                                 delegateModel.timeBeforeFromIndex(indexBefore))
        }
    }

    Action {
        id: insertAfterAction

        text: "Insert after"
        tooltip: "Insert item after this item"

        onTriggered: {
            var indexAfter = listView.indexAt(mouseArea.mouseX + listView.contentX,
                                              mouseArea.mouseY + listView.contentY)
            var indexBefore = indexAfter - 1
            delegateModel.insert(indexBefore,
                                 delegateModel.timeAfterFromIndex(indexAfter),
                                 delegateModel.timeBeforeFromIndex(indexBefore))
        }
    }

    Action {
        id: removeAction

        function deleteItemUnderCursor() {
            timeLogModel.removeItem(delegateModel.modelIndex(listView.indexAt(mouseArea.mouseX + listView.contentX,
                                                                              mouseArea.mouseY + listView.contentY)))
        }

        text: "Remove"
        tooltip: "Remove item"

        onTriggered: {
            if (recentView.isConfirmationsEnabled) {
                removeConfirmationDialog.open()
            } else {
                removeAction.deleteItemUnderCursor()
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

        onYes: removeAction.deleteItemUnderCursor()
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
        spacing: 10

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 0

            ListView {
                id: listView

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

        Item {
            Layout.fillHeight: false
            Layout.fillWidth: true
            implicitHeight: 50

            Button {
                anchors.centerIn: parent
                text: "Add item"
                tooltip: "Adds item into model"
                onClicked: {
                    var indexAfter = (listView.count === 0) ? -1 : 0
                    var indexBefore = -1
                    delegateModel.insert(indexBefore,
                                         delegateModel.timeAfterFromIndex(indexAfter),
                                         delegateModel.timeBeforeFromIndex(indexBefore))
                }
            }
        }
    }
}
