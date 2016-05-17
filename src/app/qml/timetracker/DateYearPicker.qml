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

import QtQuick 2.0
import QtQuick.Controls 2.0

ItemPositioner {
    property var beginDate: new Date()
    property var endDate: new Date()

    ComboBoxControl {
        id: periodSelector

        model: [
            qsTr("current"),
            qsTr("previous"),
            qsTr("select")
        ]

        onCurrentIndexChanged: {
            switch (currentIndex) {
            case 0:
                endDate = new Date()
                beginDate = new Date(new Date(new Date().setFullYear(endDate.getFullYear(), 0 , 1)).setHours(0, 0, 0, 0))
                break
            case 1:
                endDate = new Date(new Date(new Date().setMonth(0, 1)).setHours(0, 0, 0, 0) - 1000)
                beginDate = new Date(new Date(new Date(endDate).setMonth(0, 1)).setHours(0, 0, 0, 0))
                break
            }
        }
    }

    TextField {
        id: dateField

        visible: periodSelector.currentIndex == 2

        inputMask: "9999"
        text: beginDate.toLocaleDateString(Qt.locale(), "yyyy")

        onEditingFinished: {
            beginDate = Date.fromLocaleString(Qt.locale(), text, "yyyy")
            endDate = new Date(new Date(new Date(beginDate).setFullYear(beginDate.getFullYear() + 1, 0, 1)).setHours(0, 0, 0, 0) - 1000)
        }
    }
}
