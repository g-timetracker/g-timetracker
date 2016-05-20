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

DialogMaterial {
    id: dialog

    property alias text: label.text

    signal linkActivated(string link)

    Label {
        id: label

        Layout.fillWidth: true
        Layout.fillHeight: true
        leftPadding: 24
        rightPadding: 24
        bottomPadding: 24
        font.pixelSize: 16
        wrapMode: Text.Wrap
        color: Material.secondaryTextColor

        onLinkActivated: dialog.linkActivated(link)

        MouseArea {
            anchors.fill: parent
            cursorShape: label.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            acceptedButtons: Qt.NoButton
        }
    }
}
