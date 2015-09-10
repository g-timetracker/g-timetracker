import QtQuick 2.4
import QtQuick.Controls 1.4

TextField {
    placeholderText: "yyyy-mm-dd"
    validator: RegExpValidator {
        regExp: /^\d{4}-\d{2}-\d{2}$/
    }
}
