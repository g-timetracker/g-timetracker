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
import QtQuick.Controls 2.0
import TimeLog 1.0

ToolBarMaterial {
    property bool isBottomItem: true
    property ToolButton menuButton: ToolButton {
        id: menuButton

        font.pixelSize: 14
        contentItem: Image {
            fillMode: Image.Pad
            source: "images/ic_more_vert_white_24dp.png"

        }
        onClicked: menu.open()

        Menu {
            id: menu

            MenuItemMaterial {
                text: qsTranslate("main window", "Undo")
                iconItem.source: "images/ic_undo_white_24dp.png"
                enabled: TimeTracker.undoCount
                onTriggered: TimeTracker.undo()
            }
            MenuItemMaterial {
                text: qsTranslate("main window", "Sync")
                iconItem.source: "images/ic_sync_white_24dp.png"
                onTriggered: TimeTracker.syncRequested()
            }
        }

    }

    leftIcon: isBottomItem ? "images/ic_menu_white_24dp.png" : "images/ic_arrow_back_white_24dp.png"
    rightButtonsModel: [
        rightButton,
        menuButton
    ]

    onLeftActivated: {
        if (isBottomItem) {
            TimeTracker.openNavigationDrawerRequested()
        } else {
            TimeTracker.backRequested()
        }
    }
}
