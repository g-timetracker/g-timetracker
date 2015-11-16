import QtQuick 2.5
import Qt.labs.controls 1.0

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
