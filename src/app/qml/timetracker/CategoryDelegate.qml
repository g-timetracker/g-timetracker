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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

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
            contentItem: IconMaterial {
                visible: categoryDelegate.hasChildren
                source: "images/ic_expand_more_white_24dp.png"
                rotation: categoryDelegate.isExpanded ? 180 : 0
                Behavior on rotation {
                    animation: NumberAnimation {
                        duration: 100
                        easing.type: Easing.OutCubic
                    }
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
