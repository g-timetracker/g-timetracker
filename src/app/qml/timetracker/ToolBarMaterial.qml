import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

ToolBar {
    id: toolBar

    property alias title: titleLabel.text
    property alias leftIcon: leftIconImage.source
    property alias rightText: rightButton.text
    property alias rightContent: rightButton.contentItem
    property alias rightEnabled: rightButton.enabled

    signal leftActivated()
    signal rightActivated()

    Material.theme: Material.Dark

    ToolButton {
        Layout.fillWidth: false
        leftPadding: 16
        rightPadding: 16
        contentItem: Image {
            id: leftIconImage

            fillMode: Image.Pad
            source: "images/ic_arrow_back_white_24dp.png"
        }

        onClicked: toolBar.leftActivated()
    }

    RowLayout {
        width: parent.width
        height: toolBar.implicitHeight
        spacing: 0

        Label {
            id: titleLabel

            Layout.fillWidth: true
            leftPadding: 72
            rightPadding: rightButton.visible ? 0 : 16
            font.pixelSize: 20
            font.weight: Font.Medium
            elide: Text.ElideRight
        }

        ToolButton {
            id: rightButton

            leftPadding: 16
            rightPadding: 16
            font.pixelSize: 14
            visible: !!text
            onClicked: toolBar.rightActivated()
        }
    }
}
