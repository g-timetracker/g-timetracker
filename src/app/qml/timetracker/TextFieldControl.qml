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

Column {
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    property string helperText
    property alias inputMethodHints: textField.inputMethodHints

    spacing: 0

    TextField {
        id: textField

        topPadding: floatingLabel.height + 8
        width: parent.width
        selectByMouse: true

        LabelControl {
            id: floatingLabel

            width: parent.width
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            color: textField.activeFocus ? textField.Material.accentColor : textField.Material.hintTextColor
            text: !!textField.text ? textField.placeholderText : ""
        }
    }

    LabelControl {
        id: helperLabel

        width: parent.width
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        visible: !!helperText
        text: textField.activeFocus ? helperText : ""
    }
}
