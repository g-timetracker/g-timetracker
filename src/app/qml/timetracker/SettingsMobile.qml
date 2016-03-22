import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.2
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0
import TimeLog 1.0

Item {
    id: settingsDialog

    property string title: "Settings"

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
                text: "Enable confirmations"

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
                text: "Sync"
            }

            SettingsAdvancedDelegate {
                width: parent.width
                isLastItem: Positioner.isLastItem
                text: "Sync folder"
                additionalText: Settings.syncPath // TODO: extract path

                onClicked: syncPathDialog.open()
            }

            SettingsSwitchDelegate {
                width: parent.width
                isLastItem: Positioner.isLastItem
                text: "Auto sync"

                checked: Settings.isAutoSync
                onCheckedChanged: Settings.isAutoSync = checked
            }

            SettingsAdvancedDelegate {
                width: parent.width
                enabled: Settings.isAutoSync
                isLastItem: Positioner.isLastItem
                text: "Sync cache size"
                additionalText: Settings.syncCacheSize ? "%1 records".arg(Settings.syncCacheSize)
                                                       : "Disabled"

                onClicked: TimeTracker.showDialogRequested(syncCacheSizeDialog)
            }

            SettingsAdvancedDelegate {
                width: parent.width
                enabled: Settings.isAutoSync
                isLastItem: Positioner.isLastItem
                text: "Sync cache timeout"
                additionalText: Settings.syncCacheTimeout ? "%1 seconds".arg(Settings.syncCacheTimeout)
                                                          : "Disabled"

                onClicked: TimeTracker.showDialogRequested(syncCacheTimeoutDialog)
            }
        }
    }

    FileDialog {
        id: syncPathDialog

        title: "Select folder for sync"
        selectFolder: true
        folder: Settings.syncPath

        onAccepted: Settings.syncPath = folder
    }

    SettingsSyncCacheSize {
        id: syncCacheSizeDialog
    }

    SettingsSyncCacheTimeout {
        id: syncCacheTimeoutDialog
    }
}
