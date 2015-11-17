pragma Singleton
import QtQuick 2.0
import Qt.labs.settings 1.0

Item {
    id: settings

    property bool isConfirmationsEnabled: true
    property url syncPath
    property url dataPath

    Settings {
        property alias confirmationsEnabled: settings.isConfirmationsEnabled
        property alias syncPath: settings.syncPath
        property alias dataPath: settings.dataPath

        category: "main"
    }
}
