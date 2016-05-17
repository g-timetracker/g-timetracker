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

import QtQuick 2.5
import QtQuick.Controls 2.0

SpinBox {
    id: timeSpinBox

    property alias startDateCurrent: d.currentDate
    property alias startTimeCurrent: d.currentDateTime
    property alias minDateTime: d.minDateTime
    property alias maxDateTime: d.maxDateTime

    readonly property alias hours: d.hours
    readonly property alias minutes: d.minutes
    readonly property alias seconds: d.seconds

    from: d.minDateTime.valueOf() / 1000
    to: d.maxDateTime.valueOf() / 1000
    stepSize: 1
    editable: true

    validator: RegExpValidator {
        regExp: /([01]?[0-9]|2[0-3]):[0-5][0-9](:[0-5][0-9])?/
    }
    textFromValue: function(value, locale) {
        return new Date(value * 1000).toLocaleTimeString(locale, "h:mm:ss")
    }
    valueFromText: function(text, locale) {
        return Date.fromLocaleString(locale, "%1 %2".arg(Qt.formatDate(d.currentDate, Qt.ISODate)).arg(text),
                                     "yyyy-MM-dd h:mm%1".arg(/(\d+:){2}/.test(text) ? ":ss" : "")).valueOf() / 1000
    }

    Keys.onReturnPressed: event.accepted = true
    Keys.onEnterPressed: event.accepted = true

    TimeEditorModel {
        id: d

        hours: currentDateTime.getHours()
        minutes: currentDateTime.getMinutes()
        seconds: currentDateTime.getSeconds()
    }

    onValueChanged: {
        if (d.currentDateTime.valueOf() / 1000 !== value) {
            d.currentDateTime = new Date(value * 1000)
        }
    }

    onStartTimeCurrentChanged: {
        if (d.currentDateTime.valueOf() / 1000 !== value) {
            value = d.currentDateTime.valueOf() / 1000
        }
    }
}
