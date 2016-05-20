import QtQuick 2.6

Loader {
    property bool isLazy: true

    function get() {
        if (!active) {
            active = true
        }

        return item
    }

    active: !isLazy
}
