import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import TimeLog 1.0

ApplicationWindow {
    id: main

    title: qsTr("Hello World")
    width: 640
    height: 480
    visible: true

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: "Search"
                onTriggered: mainView.showSearch();
            }
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
                checked: Settings.isConfirmationsEnabled
                onCheckedChanged: Settings.isConfirmationsEnabled = checked
            }
        }
    }

    Connections {
        target: TimeLog
        onError: {
            errorDialog.text = errorText
            errorDialog.open()
        }
    }

    MessageDialog {
        id: errorDialog

        title: "Error"
        icon: StandardIcon.Critical
        standardButtons: StandardButton.Ok
    }

    MainView {
        id: mainView

        anchors.fill: parent
    }
}
