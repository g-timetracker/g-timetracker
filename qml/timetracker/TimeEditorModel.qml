import QtQml 2.0

QtObject {
    property date currentDate
    property var currentDateTime: new Date()
    property date startTimeAfter
    property date startTimeBefore

    property int hours
    property int minutes
    property int seconds

    property int minHours: (currentDayAfter ? startTimeAfter.getHours() + (hoursAfterFit ? 0 : 1) : 0)
    property int maxHours: (currentDayBefore ? startTimeBefore.getHours() - (hoursBeforeFit ? 0 : 1) : 23)

    property int minMinutes: (currentHourAfter ? startTimeAfter.getMinutes() + (minutesAfterFit ? 0 : 1) : 0)
    property int maxMinutes: (currentHourBefore ? startTimeBefore.getMinutes() - (minutesBeforeFit ? 0 : 1) : 59)

    property int minSeconds: (currentMinuteAfter && minutesAfterFit ? startTimeAfter.getSeconds() + 1 : 0)
    property int maxSeconds: (currentMinuteBefore && minutesBeforeFit ? startTimeBefore.getSeconds() - 1 : 59)

    property bool currentDayBefore: (currentDate.getFullYear() === startTimeBefore.getFullYear()
                                     && currentDate.getMonth() === startTimeBefore.getMonth()
                                     && currentDate.getDate() === startTimeBefore.getDate())
    property bool currentDayAfter: (startTimeAfter.getFullYear() === currentDate.getFullYear()
                                    && startTimeAfter.getMonth() === currentDate.getMonth()
                                    && startTimeAfter.getDate() === currentDate.getDate())
    property bool currentHourBefore: (currentDayBefore
                                      && hours === startTimeBefore.getHours())
    property bool currentHourAfter: (currentDayAfter
                                     && startTimeAfter.getHours() === hours)
    property bool currentMinuteBefore: (currentHourBefore
                                        && minutes === startTimeBefore.getMinutes())
    property bool currentMinuteAfter: (currentHourAfter
                                       && startTimeAfter.getMinutes() === minutes)

    property bool minutesBeforeFit: (startTimeBefore.getSeconds() > 0)
    property bool minutesAfterFit: (startTimeAfter.getSeconds() < 59)
    property bool hoursBeforeFit: (startTimeBefore.getMinutes() > 0
                                   || minutesBeforeFit)
    property bool hoursAfterFit: (startTimeAfter.getMinutes() < 59
                                  || minutesAfterFit)

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
