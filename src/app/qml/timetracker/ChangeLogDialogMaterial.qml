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

import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

DialogMaterial {
    id: dialog

    property string previousVersion
    property string currentVersion

    function checkChanges(previousVersion, currentVersion) {
        if (changeLog.checkChanges(previousVersion, currentVersion)) {
            if (AppSettings.isShowChangeLog) {
                open()
            } else {
                closed()
            }

            return true
        } else {
            return false
        }
    }

    title: qsTr("What's new")
    dismissiveText: ""

    Column {
        id: changelogColumn

        Layout.fillWidth: true
        spacing: 16

        Repeater {
            id: changelogRepeater

            delegate: Column {
                width: parent.width
                spacing: 8

                Label {
                    width: parent.width
                    font.pixelSize: 16
                    color: dialog.Material.primaryTextColor
                    text: qsTr("Version %1").arg(modelData["version"])
                }

                Label {
                    width: parent.width
                    bottomPadding: 8
                    color: dialog.Material.primaryTextColor
                    text: new Date(modelData["date"]).toLocaleDateString(Qt.locale(), Locale.ShortFormat)
                }

                Repeater {
                    model: modelData["changes"]
                    delegate: RowLayout {
                        width: parent.width
                        spacing: 0

                        Label {
                            Layout.fillWidth: false
                            Layout.alignment: Qt.AlignTop
                            color: dialog.Material.primaryTextColor
                            text: "\u2022 "
                        }
                        Label {
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                            color: dialog.Material.secondaryTextColor
                            text: modelData
                        }
                    }
                }
            }
        }

        ChangeLog {
            id: changeLog

            onChangesAvailable: changelogRepeater.model = changes
        }
    }
}
