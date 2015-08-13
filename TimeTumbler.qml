import QtQuick 2.0
import QtQuick.Extras 2.0

Row {
    id: timeTumbler

    property int hours: 0
    property int minutes: 0
    property int seconds: 0

    Tumbler {
        id: hoursTumbler

        model: 24

        onCurrentIndexChanged: {
            if (hours !== currentIndex) {
                hours = currentIndex
            }
        }
    }
    Tumbler {
        id: minutesTumbler

        model: 60

        onCurrentIndexChanged: {
            if (minutes !== currentIndex) {
                minutes = currentIndex
            }
        }
    }
    Tumbler {
        id: secondsTumbler

        model: 60

        onCurrentIndexChanged: {
            if (seconds !== currentIndex) {
                seconds = currentIndex
            }
        }
    }

    onHoursChanged: hoursTumbler.currentIndex = hours
    onMinutesChanged: minutesTumbler.currentIndex = minutes
    onSecondsChanged: secondsTumbler.currentIndex = seconds
}
