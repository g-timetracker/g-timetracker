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
