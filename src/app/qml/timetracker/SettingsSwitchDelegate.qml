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
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

SettingsDelegate {
    id: control

    indicator: Switch {
        anchors.margins: control.padding
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        enabled: control.enabled

        checked: control.checked
        onCheckedChanged: control.checked = checked
    }

    contentItem: Text {
        anchors.left: parent.left
        anchors.right: indicator.right
        anchors.leftMargin: control.padding
        anchors.rightMargin: control.spacing
        anchors.verticalCenter: parent.verticalCenter
        height: control.availableHeight
        text: control.text
        font: control.font
        color: control.enabled ? control.Material.primaryTextColor : control.Material.hintTextColor
        elide: Text.ElideRight
        visible: control.text
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}
