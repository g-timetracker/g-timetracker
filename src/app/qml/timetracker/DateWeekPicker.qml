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

import TimeLog 1.0

ItemPositioner {
    property var beginDate: new Date()
    property var endDate: new Date()

    ComboBoxControl {
        id: periodSelector

        property var weeksModel: calcModel()

        function calcModel() {
            var weeks = TimeTracker.weeksModel()
            weeks[0]["text"] = qsTr("current")
            weeks[1]["text"] = qsTr("previous")

            for (var i = 2; i < weeks.length; i++) {
                weeks[i]["text"] = qsTr("week %1").arg(weeks[i]["number"])
            }

            return weeks
        }

        model: weeksModel
        textRole: "text"

        onCurrentIndexChanged: {
            endDate = new Date(weeksModel[currentIndex]["to"].valueOf() - 1)
            beginDate = weeksModel[currentIndex]["from"]
        }
    }
}
