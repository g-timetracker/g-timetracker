import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2

ApplicationWindow {
    title: qsTr("Hello World")
    width: 640
    height: 480
    visible: true

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("&Open")
                onTriggered: messageDialog.show(qsTr("Open start triggered"));
            }
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }
    }

    TimeLogEditDialog {
        id: editDialog
    }

    TimeLogNewDialog {
        id: newDialog

        onDataAccepted: TimeLogModel.addItem(newData)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        ListView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            model: TimeLogModel

            delegate: TimeLogDelegate {
                id: delegateItem

                function updateData(category, startTime, comment) {
//                    console.log("updating", category, startTime, comment)
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

                width: parent.width
                category: model.category
                startTime: model.startTime
                durationTime: model.durationTime
                comment: model.comment

                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: {
                        editDialog.delegateItem = delegateItem
                        editDialog.open()
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
