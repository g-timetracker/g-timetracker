.pragma library

function calcDuration(timeCurrent, timeBefore) {
    return Math.ceil((timeBefore.valueOf() - timeCurrent.valueOf()) / 1000)
}

function durationText(duration) {
    var seconds, minutes, hours, days, weeks, months, years

    seconds = duration

    minutes = seconds / 60 | 0
    seconds %= 60

    hours = minutes / 60 | 0
    minutes %= 60

    days = hours / 24 | 0
    hours %= 24

    years = days / 365 | 0
    days %= 365

    months = days / 30 | 0
    days %= 30

    weeks = days / 7 | 0
    days %= 7

    var results = []
    if (years) {
        results.push("%1 years".arg(years))
    }
    if (months) {
        results.push("%1 months".arg(months))
    }
    if (weeks) {
        results.push("%1 weeks".arg(weeks))
    }
    if (days) {
        results.push("%1 days".arg(days))
    }
    if (hours) {
        results.push("%1 hours".arg(hours))
    }
    if (minutes) {
        results.push("%1 minutes".arg(minutes))
    }
    if (seconds) {
        results.push("%1 seconds".arg(seconds))
    }

    return results.join(", ")
}

function rangeList(start, end) {
    try {
        return Array.apply(null, new Array(end-start)).map(function(value, index) {
            return start + index
        })
    } catch(e) {
        console.warn("%1, start: %2, end: %3\n%4".arg(e.message).arg(start).arg(end).arg(e.stack))
        return []
    }
}
