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

Loader {
    id: changelogDialog

    function showChangeLog(previousVersion, currentVersion) {
        source = "ChangeLogDialogMaterial.qml"

        if (item.checkChanges(previousVersion, currentVersion)) {
            return true
        } else {
            changelogDialog.source = ""
            return false
        }
    }

    Connections {
        target: changelogDialog.item
        onClosed: changelogDialog.source = ""
    }
}
