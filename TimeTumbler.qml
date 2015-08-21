import QtQuick 2.0
import QtQuick.Extras 2.0
import "Util.js" as Util

Row {
    id: timeTumbler

    property var startTime: new Date()
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

        property int minHours: (currentDayAfter ? startTimeAfter.getHours() : 0)
        property int maxHours: (currentDayBefore ? startTimeBefore.getHours() : 23)

        property int minMinutes: (currentHourAfter ? startTimeAfter.getMinutes() : 0)
        property int maxMinutes: (currentHourBefore ? startTimeBefore.getMinutes() : 59)

        property int minSeconds: (currentMinuteAfter ? startTimeAfter.getSeconds() : 0)
        property int maxSeconds: (currentMinuteBefore ? startTimeBefore.getSeconds() : 59)

        property bool currentDayBefore: (startTime.getFullYear() === startTimeBefore.getFullYear()
                                         && startTime.getMonth() === startTimeBefore.getMonth()
                                         && startTime.getDate() === startTimeBefore.getDate())
        property bool currentDayAfter: (startTimeAfter.getFullYear() === startTime.getFullYear()
                                        && startTimeAfter.getMonth() === startTime.getMonth()
                                        && startTimeAfter.getDate() === startTime.getDate())
        property bool currentHourBefore: (currentDayBefore
                                          && startTime.getHours() === startTimeBefore.getHours())
        property bool currentHourAfter: (currentDayAfter
                                         && startTimeAfter.getHours() === startTime.getHours())
        property bool currentMinuteBefore: (currentHourBefore
                                            && startTime.getMinutes() === startTimeBefore.getMinutes())
        property bool currentMinuteAfter: (currentHourAfter
                                           && startTimeAfter.getMinutes() === startTime.getMinutes())
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
        setHours(startTime.getHours())
        setMinutes(startTime.getMinutes())
        setSeconds(startTime.getSeconds())
    }
}
