import QtQuick 2.0
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

import QtQuick.Controls 2.0
import "Util.js" as Util

Row {
    id: timeTumbler

    property alias startDateCurrent: d.currentDate
    property alias startTimeCurrent: d.currentDateTime
    property alias minDateTime: d.minDateTime
    property alias maxDateTime: d.maxDateTime

    readonly property alias hours: d.hours
    readonly property alias minutes: d.minutes
    readonly property alias seconds: d.seconds

    function setHours(newHours) {
        hoursTumbler.currentIndex = newHours - d.minHours
    }

    function setMinutes(newMinutes) {
        minutesTumbler.currentIndex = newMinutes - d.minMinutes
    }

    function setSeconds(newSeconds) {
        secondsTumbler.currentIndex = newSeconds - d.minSeconds
    }

    TimeEditorModel {
        id: d

        hours: hoursTumbler.currentIndex + minHours
        minutes: minutesTumbler.currentIndex + minMinutes
        seconds: secondsTumbler.currentIndex + minSeconds
    }

    Tumbler {
        id: hoursTumbler

        model: (d.minHours < d.maxHours) ? Util.rangeList(d.minHours, d.maxHours + 1)
                                         : [ d.minHours ]
    }

    Tumbler {
        id: minutesTumbler

        model: (d.minMinutes < d.maxMinutes) ? Util.rangeList(d.minMinutes, d.maxMinutes + 1)
                                             : [ d.minMinutes ]
    }

    Tumbler {
        id: secondsTumbler

        model: (d.minSeconds < d.maxSeconds) ? Util.rangeList(d.minSeconds, d.maxSeconds + 1)
                                             : [ d.minSeconds ]
    }

    onStartTimeCurrentChanged: {
        var currentTime = startTimeCurrent
        setHours(currentTime.getHours())
        setMinutes(currentTime.getMinutes())
        setSeconds(currentTime.getSeconds())
    }
}
