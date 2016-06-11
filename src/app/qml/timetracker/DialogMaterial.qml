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
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Popup {
    id: dialog

    default property alias dialogContent: contentContainer.children
    property string title: ""
    property bool customTitleBar: false
    property string affirmativeText: qsTranslate("dialog", "OK")
    property string dismissiveText: qsTranslate("dialog", "Cancel")
    property ApplicationWindow applicationWindow: PlatformMaterial.applicationWindow
    property point position: parent.mapFromItem(null,
                                                (applicationWindow.width - implicitWidth) / 2 + visible - visible,  // TODO: remove workaround
                                                (applicationWindow.height - implicitHeight) / 2)

    function accept() {
        accepted()
        close()
    }

    function reject() {
        rejected()
        close()
    }

    signal accepted()
    signal rejected()

    focus: true
    modal: false
    padding: 0
    margins: 16
    x: position.x
    y: position.y

    contentItem: ColumnLayout {
        spacing: 0

        Item {
            Layout.fillHeight: false
            Layout.fillWidth: true
            implicitHeight: 24
            visible: !dialog.customTitleBar
        }

        Item {
            Layout.fillHeight: false
            Layout.fillWidth: true
            implicitHeight: titleLabel.implicitHeight + titleLabel.anchors.bottomMargin
            implicitWidth: titleLabel.implicitWidth + titleLabel.anchors.leftMargin + titleLabel.anchors.rigthMargin
            visible: !!dialog.title

            Label {
                id: titleLabel

                anchors.leftMargin: 24
                anchors.rightMargin: 24
                anchors.bottomMargin: 20
                anchors.fill: parent
                wrapMode: Text.Wrap
                font.pixelSize: 20
                font.weight: Font.Medium
                color: dialog.Material.primaryTextColor
                text: dialog.title
            }

            Rectangle {
                width: parent.width
                height: 1
                y: parent.height - height
                color: Material.dividerColor
                visible: !contentFlickable.atYBeginning
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft
            implicitHeight: contentFlickable.contentHeight
            implicitWidth: contentFlickable.contentWidth + contentFlickable.anchors.rightMargin
                           + contentFlickable.anchors.leftMargin

            Flickable {
                id: contentFlickable

                anchors.rightMargin: 24
                anchors.leftMargin: 24
                anchors.fill: parent
                contentWidth: contentContainer.implicitWidth
                contentHeight: contentContainer.implicitHeight
                boundsBehavior: Flickable.StopAtBounds
                clip: true

                ScrollBar.vertical: ScrollBar { }

                GridLayout {
                    id: contentContainer

                    width: contentFlickable.width
                }
            }
        }

        DialogMaterialButtonBox {
            id: buttonBox

            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.minimumWidth: implicitWidth
            affirmativeText: dialog.affirmativeText
            dismissiveText: dialog.dismissiveText

            onAccepted: dialog.accept()
            onRejected: dialog.reject()

            Rectangle {
                width: parent.width
                height: 1
                y: 0
                color: Material.dividerColor
                visible: !contentFlickable.atYEnd
            }
        }
    }
}
