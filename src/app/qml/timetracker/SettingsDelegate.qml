import QtQuick 2.4
import QtQuick.Controls 2.0

ItemDelegate {
    id: settingItem

    property bool isLastItem: false

    height: 48
    font.weight: Font.Normal
    font.pixelSize: 16

    Rectangle {
        id: separator

        y: parent.height - 1
        width: parent.width
        height: 1
        color: "black"
        opacity: 0.12
        visible: !settingItem.isLastItem
    }
}
