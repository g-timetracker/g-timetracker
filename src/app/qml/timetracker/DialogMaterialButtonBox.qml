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

import QtQuick 2.5
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

Item {
    id: buttonBox

    property string affirmativeText: qsTranslate("dialog", "OK")
    property string dismissiveText: qsTranslate("dialog", "Cancel")

    signal accepted()
    signal rejected()

    implicitWidth: 56 * 5
    implicitHeight: 52

    RowLayout {
        anchors.leftMargin: 24
        anchors.rightMargin: 8
        anchors.fill: parent
        spacing: 8

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        FlatButton {
            Layout.fillWidth: false
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignVCenter
            textColor: Material.accentColor
            text: buttonBox.dismissiveText
            visible: !!text
            onClicked: buttonBox.rejected()
        }

        FlatButton {
            Layout.fillWidth: false
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignVCenter
            textColor: Material.accentColor
            text: buttonBox.affirmativeText
            visible: !!text
            onClicked: buttonBox.accepted()
        }
    }
}
