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
                text: "Sync"
                onTriggered: DataSyncer.sync(Settings.syncPath);
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
            MenuItem {
                text: "Sync path"
                onTriggered: syncPathDialog.open()
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

    Connections {
        target: DataSyncer
        onSynced: {
            messageDialog.text = "Sync complete"
            messageDialog.open()
        }
    }

    MessageDialog {
        id: messageDialog

        title: "Message"
        icon: StandardIcon.Information
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: errorDialog

        title: "Error"
        icon: StandardIcon.Critical
        standardButtons: StandardButton.Ok
    }

    FileDialog {
        id: syncPathDialog

        title: "Select folder for sync"
        selectFolder: true

        onAccepted: Settings.syncPath = folder
    }

    MainView {
        id: mainView

        anchors.fill: parent
    }
}
