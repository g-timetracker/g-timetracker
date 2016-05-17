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
import "Texts.js" as Texts

ItemPositioner {
    property var beginDate: fromField.selectedDate
    property var endDate: new Date(toField.selectedDate.valueOf() + 86399000)

    LabelControl {
        text: Texts.labelText(qsTr("From"))
    }

    DatePicker {
        id: fromField

        minimumDate: new Date(0)
        maximumDate: !!toField.selectedDate ? toField.selectedDate : new Date()
        selectedDate: maximumDate
    }

    LabelControl {
        text: Texts.labelText(qsTr("To"))
    }

    DatePicker {
        id: toField

        minimumDate: !!fromField.selectedDate ? fromField.selectedDate : new Date(0)
        maximumDate: new Date()
        selectedDate: maximumDate
    }
}
