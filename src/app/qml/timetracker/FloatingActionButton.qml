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
import QtQuick.Controls.Material.impl 2.0

Button {
    id: control

    property url iconSource

    contentItem: Image {
        fillMode: Image.Pad
        source: iconSource
    }

    Material.elevation: control.down ? 12 : 6

    background: Rectangle {
        implicitHeight: 56
        implicitWidth: 56
        radius: height / 2
        color: control.down ? control.Material.highlightedButtonPressColor
                            : control.visualFocus ? control.Material.highlightedButtonHoverColor
                                                  : control.Material.highlightedButtonColor

        layer.enabled: control.enabled
        layer.effect: ElevationEffect {
            elevation: control.Material.elevation
        }
    }
}
