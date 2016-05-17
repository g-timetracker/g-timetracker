/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

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
