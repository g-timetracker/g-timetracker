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
import QtQuick.Controls.Material 2.0
import QtGraphicalEffects 1.0

Item {
    id: iconMaterial

    property bool enabled: true
    property alias source: icon.source

    implicitHeight: icon.implicitHeight
    implicitWidth: icon.implicitWidth

    Image {
        id: icon

        anchors.centerIn: parent
        fillMode: Image.Pad
    }
    ColorOverlay {
        anchors.fill: icon
        source: icon
        color: iconMaterial.enabled ? Material.secondaryTextColor : Material.switchUncheckedTrackColor
    }
}
