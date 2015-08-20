import QtQuick 2.0
import QtQuick.Extras 2.0
import "Util.js" as Util

Row {
    id: timeTumbler

    property var startTime
    property date startTimeAfter
    property date startTimeBefore

    readonly property alias hours: d.hours
    readonly property alias minutes: d.minutes
    readonly property alias seconds: d.seconds

    QtObject {
        id: d

        property int hours: 0
        property int minutes: 0
        property int seconds: 0

        property int minHours: 0
        property int maxHours: 23

        property int minMinutes: 0
        property int maxMinutes: 59

        property int minSeconds: 0
        property int maxSeconds: 59
    }

    function recalcBoudaries() {
        if (!startTime) {
            return
        }

        var newMinHours = 0
        var newMaxHours = 23

        var newMinMinutes = 0
        var newMaxMinutes = 59

        var newMinSeconds = 0
        var newMaxSeconds = 59

        if (startTimeAfter.getFullYear() === startTimeBefore.getFullYear()
                   && startTimeAfter.getMonth() === startTimeBefore.getMonth()
                   && startTimeAfter.getDate() === startTimeBefore.getDate()) {
            newMinHours = startTimeAfter.getHours()
            newMaxHours = startTimeBefore.getHours()
            if (newMinHours === newMaxHours) {
                newMinMinutes = startTimeAfter.getMinutes()
                newMaxMinutes = startTimeBefore.getMinutes()
                if (newMaxMinutes === newMaxMinutes){
                    newMinSeconds = startTimeAfter.getSeconds()
                    newMaxSeconds = startTimeBefore.getSeconds()
                }
            }
        } else if (startTime.getFullYear() === startTimeBefore.getFullYear()
                   && startTime.getMonth() === startTimeBefore.getMonth()
                   && startTime.getDate() === startTimeBefore.getDate()) {
            newMinHours = 0
            newMaxHours = startTimeBefore.getHours()
            if (startTime.getHours() === startTimeBefore.getHours()) {
                newMinMinutes = 0
                newMaxMinutes = startTimeBefore.getMinutes()
                if (startTime.getMinutes() === startTimeBefore.getMinutes()) {
                    newMinSeconds = 0
                    newMaxSeconds = startTimeBefore.getSeconds()
                }
            }
        } else if (startTimeAfter.getFullYear() === startTime.getFullYear()
                   && startTimeAfter.getMonth() === startTime.getMonth()
                   && startTimeAfter.getDate() === startTime.getDate()) {
            newMinHours = startTimeAfter.getHours()
            newMaxHours = 23
            if (startTimeAfter.getHours() === startTime.getHours()) {
                newMinMinutes = startTimeAfter.getMinutes()
                newMaxMinutes = 59
                if (startTimeAfter.getMinutes() === startTime.getMinutes()) {
                    newMinSeconds = startTimeAfter.getSeconds()
                    newMaxSeconds = 59
                }
            }
        }

        if (d.minHours !== newMinHours || d.maxHours !== newMaxHours) {
            d.minHours = newMinHours
            d.maxHours = newMaxHours
            hoursTumbler.model = Util.rangeList(d.minHours, d.maxHours + 1)
        }
        if (d.minMinutes !== newMinMinutes || d.maxMinutes !== newMaxMinutes) {
            d.minMinutes = newMinMinutes
            d.maxMinutes = newMaxMinutes
            minutesTumbler.model = Util.rangeList(d.minMinutes, d.maxMinutes + 1)
        }
        if (d.minSeconds !== newMinSeconds || d.maxSeconds !== newMaxSeconds) {
            d.minSeconds = newMinSeconds
            d.maxSeconds = newMaxSeconds
            secondsTumbler.model = Util.rangeList(d.minSeconds, d.maxSeconds + 1)
        }
    }

    function setHours(newHours) {
        hoursTumbler.currentIndex = newHours - d.minHours
    }

    function setMinutes(newMinutes) {
        minutesTumbler.currentIndex = newMinutes - d.minMinutes
    }

    function setSeconds(newSeconds) {
        secondsTumbler.currentIndex = newSeconds - d.minSeconds
    }

    Tumbler {
        id: hoursTumbler

        model: Util.rangeList(d.minHours, d.maxHours + 1)

        onCurrentIndexChanged: {
            if (d.hours !== currentIndex + d.minHours) {
                d.hours = currentIndex + d.minHours
            }
        }
    }
    Tumbler {
        id: minutesTumbler

        model: Util.rangeList(d.minMinutes, d.maxMinutes + 1)

        onCurrentIndexChanged: {
            if (d.minutes !== currentIndex + d.minMinutes) {
                d.minutes = currentIndex + d.minMinutes
            }
        }
    }
    Tumbler {
        id: secondsTumbler

        model: Util.rangeList(d.minSeconds, d.maxSeconds + 1)

        onCurrentIndexChanged: {
            if (d.seconds !== currentIndex + d.minSeconds) {
                d.seconds = currentIndex + d.minSeconds
            }
        }
    }

    onStartTimeChanged: {
        if (!startTime) {
            return
        }

        recalcBoudaries()
        setHours(startTime.getHours())
        setMinutes(startTime.getMinutes())
        setSeconds(startTime.getSeconds())
    }
    onStartTimeAfterChanged: recalcBoudaries()
    onStartTimeBeforeChanged: recalcBoudaries()

    onHoursChanged: {
        if (startTime) {
            startTime.setHours(d.hours)
            startTime = startTime
        }
    }

    onMinutesChanged: {
        if (startTime) {
            startTime.setMinutes(d.minutes)
            startTime = startTime
        }
    }

    onSecondsChanged: {
        if (startTime) {
            startTime.setSeconds(d.seconds)
            startTime = startTime
        }
    }
}
