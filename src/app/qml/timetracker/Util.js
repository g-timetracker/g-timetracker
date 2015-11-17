.pragma library

function calcDuration(timeCurrent, timeBefore) {
    return Math.ceil((timeBefore.valueOf() - timeCurrent.valueOf()) / 1000)
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

function shuffleArray(array) {
    var current = array ? array.length : 0
    var index, tmp

    while (current) {
        index = Math.floor(Math.random() * current--)

        tmp = array[current]
        array[current] = array[index]
        array[index] = tmp
    }

    return array
}
