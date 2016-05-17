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

import QtQml 2.0

QtObject {
    property date currentDate
    property var currentDateTime: new Date()
    property date minDateTime
    property date maxDateTime

    property int hours
    property int minutes
    property int seconds

    property int minHours: (isCurrentDayMin ? minDateTime.getHours() : 0)
    property int maxHours: (isCurrentDayMax ? maxDateTime.getHours() : 23)

    property int minMinutes: (isCurrentHourMin ? minDateTime.getMinutes() : 0)
    property int maxMinutes: (isCurrentHourMax ? maxDateTime.getMinutes() : 59)

    property int minSeconds: (isCurrentMinuteMin ? minDateTime.getSeconds() : 0)
    property int maxSeconds: (isCurrentMinuteMax ? maxDateTime.getSeconds() : 59)

    property bool isCurrentDayMin: (minDateTime.getFullYear() === currentDate.getFullYear()
                                    && minDateTime.getMonth() === currentDate.getMonth()
                                    && minDateTime.getDate() === currentDate.getDate())
    property bool isCurrentDayMax: (currentDate.getFullYear() === maxDateTime.getFullYear()
                                    && currentDate.getMonth() === maxDateTime.getMonth()
                                    && currentDate.getDate() === maxDateTime.getDate())
    property bool isCurrentHourMin: (isCurrentDayMin
                                     && minDateTime.getHours() === hours)
    property bool isCurrentHourMax: (isCurrentDayMax
                                     && hours === maxDateTime.getHours())
    property bool isCurrentMinuteMin: (isCurrentHourMin
                                       && minDateTime.getMinutes() === minutes)
    property bool isCurrentMinuteMax: (isCurrentHourMax
                                       && minutes === maxDateTime.getMinutes())

    onHoursChanged: {
        if (currentDateTime.getHours() != hours) {
            currentDateTime.setHours(hours)
            currentDateTime = currentDateTime
        }
    }

    onMinutesChanged: {
        if (currentDateTime.getMinutes() != minutes) {
            currentDateTime.setMinutes(minutes)
            currentDateTime = currentDateTime
        }
    }

    onSecondsChanged: {
        if (currentDateTime.getSeconds() != seconds) {
            currentDateTime.setSeconds(seconds)
            currentDateTime = currentDateTime
        }
    }

    onCurrentDateChanged: {
        var newDate = new Date(currentDate)
        currentDateTime.setFullYear(newDate.getFullYear())
        currentDateTime.setMonth(newDate.getMonth())
        currentDateTime.setDate(newDate.getDate())
        currentDateTime = currentDateTime
    }
}
