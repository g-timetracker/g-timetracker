import QtQuick 2.0
import QtQuick.Extras 2.0
import "Util.js" as Util

Row {
    id: timeTumbler

    property int hours: 0
    property int minutes: 0
    property int seconds: 0

    property int minHours: 0
    property int maxHours: 23

    property int minMinutes: 0
    property int maxMinutes: 59

    property int minSeconds: 0
    property int maxSeconds: 59

    property var hoursModel: Util.rangeList(minHours, maxHours + 1)
    property var minutesModel: Util.rangeList(minMinutes, maxMinutes + 1)
    property var secondsModel: Util.rangeList(minSeconds, maxSeconds + 1)


    Tumbler {
        id: hoursTumbler

        model: hoursModel

        onCurrentIndexChanged: {
            if (hours !== currentIndex + minHours) {
                hours = currentIndex + minHours
            }
        }
    }
    Tumbler {
        id: minutesTumbler

        model: minutesModel

        onCurrentIndexChanged: {
            if (minutes !== currentIndex + minMinutes) {
                minutes = currentIndex + minMinutes
            }
        }
    }
    Tumbler {
        id: secondsTumbler

        model: secondsModel

        onCurrentIndexChanged: {
            if (seconds !== currentIndex + minSeconds) {
                seconds = currentIndex + minSeconds
            }
        }
    }

    onHoursChanged: hoursTumbler.currentIndex = hours - minHours
    onMinutesChanged: minutesTumbler.currentIndex = minutes - minMinutes
    onSecondsChanged: secondsTumbler.currentIndex = seconds - minSeconds
}
