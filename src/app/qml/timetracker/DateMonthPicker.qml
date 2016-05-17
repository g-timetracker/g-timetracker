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

ItemPositioner {
    property var beginDate: new Date()
    property var endDate: new Date()

    ComboBoxControl {
        id: periodSelector

        function calcModel() {
            var months = []
            for (var i = 0; i < 12; i++) {
                months.push(Qt.locale().standaloneMonthName(i, Locale.LongFormat))
            }

            var result = new Array(12)
            var start = new Date().getMonth();
            for (var count = 0; count < 12; count++) {
                switch (count) {
                case 0:
                    result[count] = qsTr("current")
                    break
                case 1:
                    result[count] = qsTr("previous")
                    break
                default:
                    result[count] = months[(start - count + 12) % 12]
                    break
                }
            }

            return result
        }

        model: calcModel()

        onCurrentIndexChanged: {
            switch (currentIndex) {
            case 0:
                endDate = new Date()
                beginDate = new Date(new Date(new Date(endDate).setDate(1)).setHours(0, 0, 0, 0))
                break
            default:
                endDate = new Date(new Date(new Date().setMonth(new Date().getMonth() - currentIndex + 1, 1)).setHours(0, 0, 0, 0) - 1000)
                beginDate = new Date(new Date(new Date(endDate).setDate(1)).setHours(0, 0, 0, 0))
                break
            }
        }
    }
}
