import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.2
import QtQml.Models 2.2
import Qt.labs.settings 1.0

ApplicationWindow {
    id: main

    property bool isConfirmationsEnabled: true

    title: qsTr("Hello World")
    width: 640
    height: 480
    visible: true

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }
        Menu {
            title: "Settings"
            MenuItem {
                text: "Confirmations"
                checkable: true
                checked: main.isConfirmationsEnabled
                onCheckedChanged: main.isConfirmationsEnabled = checked
            }
        }
    }

    Settings {
        property alias confirmationsEnabled: main.isConfirmationsEnabled

        category: "main"
    }

    TimeLogEditDialog {
        id: editDialog
    }

    TimeLogNewDialog {
        id: newDialog

        onDataAccepted: {
            if (newDialog.indexBefore === -1) {
                TimeLogModel.appendItem(newData)
            } else {
                TimeLogModel.insertItem(delegateModel.modelIndex(newDialog.indexBefore), newData)
            }
        }
    }

    DelegateModel {
        id: delegateModel

        model: TimeLogModel
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

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        ListView {
            id: listView

            function deleteItemUnderCursor() {
                TimeLogModel.removeItem(delegateModel.modelIndex(listView.indexAt(mouseArea.mouseX,
                                                                                  mouseArea.mouseY)))
            }

            Layout.fillHeight: true
            Layout.fillWidth: true
            model: delegateModel

            Action {
                id: editAction

                text: "Edit"
                tooltip: "Edit item"

                onTriggered: {
                    editDialog.delegateItem = listView.itemAt(mouseArea.mouseX, mouseArea.mouseY)
                    editDialog.open()
                }
            }

            Action {
                id: insertBeforeAction

                text: "Insert before"
                tooltip: "Insert item before this item"

                onTriggered: {
                    var indexBefore = listView.indexAt(mouseArea.mouseX, mouseArea.mouseY)
                    var startTimeBefore = TimeLogModel.timeLogData(delegateModel.modelIndex(indexBefore)).startTime
                    var indexAfter = indexBefore - 1
                    var startTimeAfter
                    if (indexAfter === -1) {
                        startTimeAfter = new Date(0)
                    } else {
                        startTimeAfter = TimeLogModel.timeLogData(delegateModel.modelIndex(indexAfter)).startTime
                    }
                    newDialog.openDialog(indexBefore, startTimeAfter, startTimeBefore)
                }
            }

            Action {
                id: insertAfterAction

                text: "Insert after"
                tooltip: "Insert item after this item"

                onTriggered: {
                    var indexAfter = listView.indexAt(mouseArea.mouseX, mouseArea.mouseY)
                    var startTimeAfter = TimeLogModel.timeLogData(delegateModel.modelIndex(indexAfter)).startTime
                    var indexBefore = (indexAfter + 1 === listView.count) ? -1 : indexAfter + 1
                    var startTimeBefore
                    if (indexBefore === -1) {
                        startTimeBefore = new Date()
                    } else {
                        startTimeBefore = TimeLogModel.timeLogData(delegateModel.modelIndex(indexBefore)).startTime
                    }
                    newDialog.openDialog(indexBefore, startTimeAfter, startTimeBefore)
                }
            }

            Action {
                id: removeAction

                text: "Remove"
                tooltip: "Remove item"

                onTriggered: {
                    if (main.isConfirmationsEnabled) {
                        removeConfirmationDialog.open()
                    } else {
                        listView.deleteItemUnderCursor()
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

                onYes: listView.deleteItemUnderCursor()
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

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onDoubleClicked: {
                    if (listView.itemAt(mouse.x, mouse.y)) {
                        editAction.trigger()
                    }
                }

                onClicked: {
                    if (mouse.button === Qt.RightButton && listView.itemAt(mouse.x, mouse.y)) {
                        itemMenu.popup()
                    }
                }
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
                    var indexAfter = listView.count - 1
                    var startTimeAfter
                    if (indexAfter === -1) {
                        startTimeAfter = new Date(0)
                    } else {
                        startTimeAfter = TimeLogModel.timeLogData(delegateModel.modelIndex(indexAfter)).startTime
                    }
                    var startTimeBefore = new Date()
                    newDialog.openDialog(-1, startTimeAfter, startTimeBefore)
                }
            }
        }
    }
}
