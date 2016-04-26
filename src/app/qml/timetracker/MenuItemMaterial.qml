import QtQuick.Controls 2.0

MenuItem {
    id: control

    property alias iconItem: icon

    leftPadding: 56

    indicator: IconMaterial {
        id: icon

        x: 16
        y: control.topPadding + (control.availableHeight - height) / 2
        enabled: control.enabled
    }
}
