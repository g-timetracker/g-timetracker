import QtQuick 2.0
import Qt.labs.controls 1.0
import "Util.js" as Util

Row {
    id: timeTumbler

    property alias startDateCurrent: d.currentDate
    property alias startTimeCurrent: d.currentDateTime
    property alias startTimeAfter: d.startTimeAfter
    property alias startTimeBefore: d.startTimeBefore

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
        setHours(startTimeCurrent.getHours())
        setMinutes(startTimeCurrent.getMinutes())
        setSeconds(startTimeCurrent.getSeconds())
    }
}
