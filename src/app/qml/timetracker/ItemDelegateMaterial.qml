import QtQuick.Controls 2.0

ItemDelegate {
    id: control

    property alias iconItem: icon

    leftPadding: 72

    indicator: IconMaterial {
        id: icon

        x: 16
        y: control.topPadding + (control.availableHeight - height) / 2
        enabled: control.enabled
    }
}
