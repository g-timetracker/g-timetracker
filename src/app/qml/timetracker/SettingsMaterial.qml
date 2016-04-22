import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Item {
    id: settingsDialog

    property string title: qsTranslate("main window", "Settings")

    function close() {
        TimeTracker.backRequested()
    }

    Flickable {
        anchors.bottomMargin: Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio
        anchors.fill: parent
        contentWidth: settingsColumn.implicitWidth
        contentHeight: settingsColumn.implicitHeight
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { }

        Column {
            id: settingsColumn

            width: settingsDialog.width

            SettingsSwitchDelegate {
                width: parent.width
                isLastItem: Positioner.isLastItem
                text: qsTranslate("settings", "Confirmations")

                checked: Settings.isConfirmationsEnabled
                onCheckedChanged: Settings.isConfirmationsEnabled = checked
            }

            Label {
                height: 48
                anchors.left: parent.left
                anchors.leftMargin: 16
                font.weight: Font.Medium
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
                color: Material.accentColor
                text: qsTranslate("settings", "Sync")
            }

            SettingsAdvancedDelegate {
                width: parent.width
                isLastItem: Positioner.isLastItem
                text: qsTranslate("settings", "Sync folder")
                additionalText: !!Settings.syncPath.toString() ? (Settings.syncPath.toString().replace(/file:\/\//, ""))
                                                               : qsTranslate("settings", "Not set")

                onClicked: syncPathDialog.open()
            }

            SettingsSwitchDelegate {
                width: parent.width
                isLastItem: Positioner.isLastItem
                text: qsTranslate("settings", "Auto sync")

                checked: Settings.isAutoSync
                onCheckedChanged: Settings.isAutoSync = checked
            }

            SettingsAdvancedDelegate {
                width: parent.width
                enabled: Settings.isAutoSync
                isLastItem: Positioner.isLastItem
                text: qsTranslate("settings", "Sync cache size")
                additionalText: Settings.syncCacheSize ? qsTranslate("settings", "%n record(s)",
                                                                     "current sync cache size",
                                                                     Settings.syncCacheSize)
                                                       : qsTranslate("settings", "Disabled")

                onClicked: TimeTracker.showDialogRequested(syncCacheSizeDialog)
            }

            SettingsAdvancedDelegate {
                width: parent.width
                enabled: Settings.isAutoSync
                isLastItem: Positioner.isLastItem
                text: qsTranslate("settings", "Sync timeout")
                additionalText: Settings.syncCacheTimeout ? qsTranslate("settings", "%n second(s)",
                                                                        "current sync cache timeout",
                                                                        Settings.syncCacheTimeout)
                                                          : qsTranslate("settings", "Disabled")

                onClicked: TimeTracker.showDialogRequested(syncCacheTimeoutDialog)
            }
        }
    }

    SyncFolderDialog {
        id: syncPathDialog

        folder: !!Settings.syncPath.toString() ? Settings.syncPath : TimeTracker.documentsLocation()

        onAccepted: Settings.syncPath = folder
    }

    SettingsSyncCacheSize {
        id: syncCacheSizeDialog
    }

    SettingsSyncCacheTimeout {
        id: syncCacheTimeoutDialog
    }
}
