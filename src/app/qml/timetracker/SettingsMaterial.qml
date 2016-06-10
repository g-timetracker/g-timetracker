/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: settingsDialog

    title: qsTranslate("main window", "Settings")

    header: MainToolBarMaterial {
        title: settingsDialog.title
        isBottomItem: settingsDialog.StackView.index === 0
        rightButtonsModel: [
            rightButton
        ]
    }

    function close() {
        TimeTracker.backRequested()
    }

    function openSyncFolderDialog() {
        syncPathDialog.get().open()
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
                text: qsTranslate("settings", "Check for updates")
                visible: PlatformMaterial.isDesktop

                checked: AppSettings.isCheckForUpdates
                onCheckedChanged: AppSettings.isCheckForUpdates = checked
            }

            SettingsSwitchDelegate {
                width: parent.width
                isLastItem: Positioner.isLastItem
                text: qsTranslate("settings", "Confirmations")

                checked: AppSettings.isConfirmationsEnabled
                onCheckedChanged: AppSettings.isConfirmationsEnabled = checked
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
                additionalText: TimeTracker.pathToNativeSeparators(TimeTracker.urlToLocalFile(TimeTracker.syncer.syncPath))
                                || qsTranslate("settings", "Not set")

                onClicked: syncPathDialog.get().open()
            }

            SettingsSwitchDelegate {
                width: parent.width
                isLastItem: Positioner.isLastItem
                text: qsTranslate("settings", "Auto sync")

                checked: AppSettings.isAutoSync
                onCheckedChanged: AppSettings.isAutoSync = checked
            }

            SettingsAdvancedDelegate {
                width: parent.width
                enabled: AppSettings.isAutoSync
                isLastItem: Positioner.isLastItem
                text: qsTranslate("settings", "Sync cache size")
                additionalText: AppSettings.syncCacheSize ? qsTranslate("settings", "%n record(s)",
                                                                        "current sync cache size",
                                                                        AppSettings.syncCacheSize)
                                                          : qsTranslate("settings", "Disabled")

                onClicked: TimeTracker.showDialogRequested(syncCacheSizeDialog.get())
            }

            SettingsAdvancedDelegate {
                width: parent.width
                enabled: AppSettings.isAutoSync
                isLastItem: Positioner.isLastItem
                text: qsTranslate("settings", "Sync timeout")
                additionalText: AppSettings.syncCacheTimeout ? qsTranslate("settings", "%n second(s)",
                                                                           "current sync cache timeout",
                                                                           AppSettings.syncCacheTimeout)
                                                             : qsTranslate("settings", "Disabled")

                onClicked: TimeTracker.showDialogRequested(syncCacheTimeoutDialog.get())
            }
        }
    }

    LazyLoader {
        id: syncPathDialog

        sourceComponent: SyncFolderDialog {
            folder: !!AppSettings.syncPath.toString() ? AppSettings.syncPath
                                                      : TimeTracker.documentsLocation()
        }
    }

    LazyLoader {
        id: syncCacheSizeDialog

        sourceComponent: SettingsSyncCacheSize { }
    }

    LazyLoader {
        id: syncCacheTimeoutDialog

        sourceComponent: SettingsSyncCacheTimeout { }
    }
}
