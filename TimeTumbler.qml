import QtQuick 2.0
import QtQuick.Extras 1.4

Tumbler {
    id: timeTumbler

    property int hours: 0
    property int minutes: 0
    property int seconds: 0

    TumblerColumn {
        model: 24

        onCurrentIndexChanged: {
            if (hours !== currentIndex) {
                hours = currentIndex
            }
        }
    }
    TumblerColumn {
        model: 60

        onCurrentIndexChanged: {
            if (minutes !== currentIndex) {
                minutes = currentIndex
            }
        }
    }
    TumblerColumn {
        model: 60

        onCurrentIndexChanged: {
            if (seconds !== currentIndex) {
                seconds = currentIndex
            }
        }
    }

    onHoursChanged: setCurrentIndexAt(0, hours)
    onMinutesChanged: setCurrentIndexAt(1, minutes)
    onSecondsChanged: setCurrentIndexAt(2, seconds)
}
