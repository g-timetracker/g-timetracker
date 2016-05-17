pragma Singleton
import QtQuick 2.0
import Qt.labs.settings 1.0

Item {
    id: settings

    property bool isConfirmationsEnabled: true
    property string lastVersion
    property bool isAutoSync: true
    property int syncCacheSize: 10
    property int syncCacheTimeout: 3600
    property url syncPath
    property url dataPath

    Settings {
        property alias confirmationsEnabled: settings.isConfirmationsEnabled
        property alias lastVersion: settings.lastVersion

        category: "main"
    }

    Settings {
        property alias autoSync: settings.isAutoSync
        property alias syncCacheSize: settings.syncCacheSize
        property alias syncCacheTimeout: settings.syncCacheTimeout
        property alias syncPath: settings.syncPath
        property alias dataPath: settings.dataPath

        category: "sync"
    }
}
