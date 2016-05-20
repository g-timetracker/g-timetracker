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

import QtQuick 2.0
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQml.Models 2.2
import TimeLog 1.0

Item {
    property string title: qsTranslate("main window", "Categories")

    QtObject {
        id: d

        property bool isRemoveEnabled: treeView.isCurrentIndexValid
                                       && !categoryModel.data(treeView.selection.currentIndex,
                                                              TimeLogCategoryTreeModel.HasItemsRole)
        property bool isShowEntriesEnabled: treeView.isCurrentIndexValid
        property bool isShowStatsEnabled: treeView.isCurrentIndexValid

        function remove() {
            if (AppSettings.isConfirmationsEnabled) {
                removeConfirmationDialog.open()
            } else {
                categoryModel.removeItem(treeView.selection.currentIndex)
            }
        }

        function showEntries() {
            TimeTracker.showSearchRequested(categoryModel.data(treeView.selection.currentIndex,
                                                               TimeLogCategoryTreeModel.FullNameRole))
        }

        function showStats() {
            TimeTracker.showStatsRequested(categoryModel.data(treeView.selection.currentIndex,
                                                              TimeLogCategoryTreeModel.FullNameRole))
        }
    }

    TimeLogCategoryTreeModel {
        id: categoryModel

        timeTracker: TimeTracker
    }

    RemoveConfirmationDialog {
        id: removeConfirmationDialog

        text: qsTranslate("categories view", "Delete this category?")

        onAccepted: categoryModel.removeItem(treeView.selection.currentIndex)
    }

    Menu {
        id: itemMenu

        function popup() {
            x = mouseArea.mouseX
            y = mouseArea.mouseY
            open()
        }

        MenuItem {
            text: qsTranslate("categories view", "Delete")
            enabled: d.isRemoveEnabled
            onTriggered: d.remove()
        }
        MenuItem {
            text: qsTranslate("categories view", "Show entries")
            enabled: d.isShowEntriesEnabled
            onTriggered: d.showEntries()
        }
        MenuItem {
            text: qsTranslate("categories view", "Show statistics")
            enabled: d.isShowStatsEnabled
            onTriggered: d.showStats()
        }
    }

    ColumnLayout {
        anchors.fill: parent

        QQC1.TreeView {
            id: treeView

            property bool isCurrentIndexValid: selection.currentIndex.valid

            Layout.fillWidth: true
            Layout.fillHeight: true
            model: categoryModel
            headerVisible: true
            selectionMode: QQC1.SelectionMode.SingleSelection
            selection: ItemSelectionModel {
                model: categoryModel
            }

            QQC1.TableViewColumn {
                role: "name"
            }

            QQC1.TableViewColumn {
                role: "fullName"
            }

            QQC1.TableViewColumn {
                role: "comment"
            }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                propagateComposedEvents: true

                onClicked: {
                    var index = treeView.indexAt(mouse.x, mouse.y)
                    if (index.valid) {
                        treeView.selection.setCurrentIndex(index, ItemSelectionModel.SelectCurrent)
                        itemMenu.popup()
                    }
                }
            }

            Keys.onPressed: {
                if (event.key === Qt.Key_F2 && d.isEditEnabled) {
                    d.edit()
                }
            }

            onPressAndHold: itemMenu.popup()
            onDoubleClicked: d.showEntries()
        }

        Row {
            Layout.fillWidth: true
            Layout.fillHeight: false
            spacing: 10

            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: d.isRemoveEnabled
                text: qsTranslate("categories view", "Delete")
                onClicked: d.remove()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: d.isShowEntriesEnabled
                text: qsTranslate("categories view", "Show entries")
                onClicked: d.showEntries()
            }
            PushButton {
                anchors.verticalCenter: parent.verticalCenter
                enabled: d.isShowStatsEnabled
                text: qsTranslate("categories view", "Show statistics")
                onClicked: d.showStats()
            }
        }
    }
}
