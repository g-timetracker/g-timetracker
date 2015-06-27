import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2

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

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        ListView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            model: TimeLogModel
            delegate: Rectangle {
                width: parent.width
                height: 50
                border {
                    width: 1
                    color: "black"
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: parent.border.width
                    spacing: 10

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: false
                        implicitHeight: startTimeText.implicitHeight
                        implicitWidth: startTimeText.implicitWidth

                        Text {
                            id: startTimeText

                            anchors.centerIn: parent
                            text: model.startTime
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: false
                        implicitHeight: durationTimeText.implicitHeight
                        implicitWidth: durationTimeText.implicitWidth

                        Text {
                            id: durationTimeText

                            anchors.centerIn: parent
                            text: model.durationTime
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: false
                        implicitHeight: categoryTimeText.implicitHeight
                        implicitWidth: categoryTimeText.implicitWidth

                        Text {
                            id: categoryTimeText

                            anchors.centerIn: parent
                            text: model.category
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        implicitHeight: commentTimeText.implicitHeight
                        implicitWidth: commentTimeText.implicitWidth

                        Text {
                            id: commentTimeText

                            anchors.centerIn: parent
                            text: model.comment
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: {
                        model.comment = "Edited!"
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
                onClicked: TimeLogModel.addItem()
            }
        }
    }
}
