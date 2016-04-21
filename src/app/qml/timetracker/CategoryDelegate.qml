import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0

ItemDelegate {
    id: categoryDelegate

    property string name
    property string fullName
    property var categoryData
    property string comment: categoryData && categoryData.comment ? categoryData.comment : ""
    property bool hasChildren: false
    property bool hasItems: false
    property bool isExpanded: false
    property int depth: 0
    property bool isCurrent: false
    property bool isLast: false

    function updateData(category) {
        if (categoryDelegate.fullName !== category.name
            || !((!categoryDelegate.comment && !category.data.comment)
                 || categoryDelegate.comment === category.data.comment)) {
            model.category = category
        }
    }

    signal expand()
    signal collapse()

    implicitHeight: 72
    padding: 0
    spacing: 0
    down: isCurrent

    contentItem: RowLayout {
        anchors.leftMargin: 16 + Math.min(categoryDelegate.depth, 2) * 32
        anchors.rightMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 16

        Column {
            Layout.fillWidth: true
            spacing: 4

            Label {
                id: categoryLabel

                width: parent.width
                elide: Text.ElideRight
                font.pixelSize: 16
                color: categoryDelegate.isExpanded ? Material.accentColor : Material.primaryTextColor
                text: categoryDelegate.name
            }

            Label {
                width: parent.width
                elide: Text.ElideRight
                font.pixelSize: 14
                color: Material.secondaryTextColor
                visible: !!text
                text: categoryDelegate.comment
            }
        }

        ToolButton {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.fillWidth: false
            contentItem: Item {
                visible: categoryDelegate.hasChildren
                rotation: categoryDelegate.isExpanded ? 180 : 0
                Behavior on rotation {
                    animation: NumberAnimation {
                        duration: 100
                        easing.type: Easing.OutCubic
                    }
                }

                Image {
                    id: icon

                    anchors.centerIn: parent
                    fillMode: Image.Pad
                    source: "images/ic_expand_more_white_24dp.png"
                }
                ColorOverlay {
                    anchors.fill: icon
                    source: icon
                    color: Material.secondaryTextColor
                }
            }

            onClicked: {
                if (categoryDelegate.isExpanded) {
                    categoryDelegate.collapse()
                } else {
                    categoryDelegate.expand()
                }
            }
        }
    }

    Rectangle {
        width: parent.width
        height: 1
        y: parent.height - height
        color: Material.dividerColor
        visible: !categoryDelegate.isLast
    }
}
