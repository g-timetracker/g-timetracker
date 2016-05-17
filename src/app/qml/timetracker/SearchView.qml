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

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: view

    property alias category: timeLogFilter.category

    title: qsTranslate("main window", "Search")

    header: MainToolBarMaterial {
        title: view.title
        isBottomItem: view.StackView.index === 0
        rightText: "customize"
        rightContent: Image {
            fillMode: Image.Pad
            source: "images/ic_tune_white_24dp.png"
        }

        onRightActivated: rightDrawer.open()
    }

    TimeLogSearchModel {
        id: timeLogModel

        timeTracker: TimeTracker
        begin: timeLogFilter.beginDate
        end: timeLogFilter.endDate
        category: timeLogFilter.category
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        Label {
            Layout.fillHeight: false
            Layout.fillWidth: true
            topPadding: 16
            leftPadding: 32
            rightPadding: 32
            bottomPadding: 0
            font.pixelSize: 16
            color: Material.hintTextColor
            wrapMode: Text.Wrap
            text: qsTranslate("search filter", "Selected period: %1\u2013%2")
                  .arg(timeLogFilter.beginDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat))
                  .arg(timeLogFilter.endDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat))
        }

        Label {
            Layout.fillHeight: false
            Layout.fillWidth: true
            padding: 0
            leftPadding: 32
            rightPadding: 32
            font.pixelSize: 16
            color: Material.hintTextColor
            wrapMode: Text.Wrap
            visible: !!timeLogFilter.category
            text: qsTranslate("search filter", "Category: \u201C%1\u201D").arg(timeLogFilter.category)
        }

        TimeLogView {
            id: timeLogView

            function showHistory() {
                var item = timeLogView.currentItem
                var beginDate = new Date(Math.max(item.startTime.valueOf() - 6 * 60 * 60 * 1000, 0))
                var endDate = new Date(Math.min(item.succeedingStart.valueOf() - 1000 + 6 * 60 * 60 * 1000, Date.now()))
                TimeTracker.showHistoryRequested(beginDate, endDate)
            }

            property MenuItem showHistoryMenuItem: MenuItemMaterial {
                text: qsTr("Show in history")
                iconItem.source: "images/ic_history_white_24dp.png"
                onTriggered: timeLogView.showHistory()
            }
            property ItemDelegate showHistoryBottomSheetItem: ItemDelegateMaterial {
                width: timeLogView.width
                text: qsTr("Show in history")
                iconItem.source: "images/ic_history_white_24dp.png"
                onClicked: {
                    timeLogView.showHistory()
                    timeLogView.closeBottomSheet()
                }
            }

            Layout.fillHeight: true
            Layout.fillWidth: true
            model: timeLogModel
            menuModel: [
                showHistoryMenuItem,
                timeLogView.editMenuItem
            ]
            bottomSheetModel: [
                showHistoryBottomSheetItem,
                timeLogView.editBottomSheetItem
            ]
        }
    }

    Drawer {
        id: rightDrawer

        height: parent.height + view.header.height
        implicitWidth: Math.min(timeLogFilter.implicitWidth, parent.width - 56)
        edge: Qt.RightEdge

        Flickable {
            anchors.fill: parent
            contentWidth: timeLogFilter.implicitWidth
            contentHeight: timeLogFilter.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar { }

            TimeLogFilter {
                id: timeLogFilter

                padding: 16
                topPadding: 8
                bottomPadding: 8
            }
        }
    }
}
