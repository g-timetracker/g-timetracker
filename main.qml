import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
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
        anchors.fill: parent
        isConfirmationsEnabled: main.isConfirmationsEnabled
    }
}
