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

        onDataAccepted: TimeLogModel.addItem(newData)
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
                onClicked: newDialog.open()
            }
        }
    }
}
