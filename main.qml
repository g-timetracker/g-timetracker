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

            delegate: TimeLogDelegate {
                width: parent.width
                category: model.category
                startTime: model.startTime
                durationTime: model.durationTime
                comment: model.comment

                onUpdateCategory: model.category = newCategory
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
