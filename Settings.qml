pragma Singleton
import QtQuick 2.0
import Qt.labs.settings 1.0

Item {
    id: settings

    property bool isConfirmationsEnabled: true

    Settings {
        property alias confirmationsEnabled: settings.isConfirmationsEnabled

        category: "main"
    }
}
