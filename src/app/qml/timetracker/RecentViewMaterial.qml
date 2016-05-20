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
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: view

    title: qsTranslate("main window", "Recent")

    header: MainToolBarMaterial {
        title: view.title
        isBottomItem: view.StackView.index === 0

        ProgressBar {
            width: parent.width
            y: parent.height - height
            indeterminate: true
            visible: TimeTracker.syncer ? TimeTracker.syncer.isRunning : false
        }
    }

    ReverseProxyModel {
        id: timeLogModel

        sourceModel: TimeLogRecentModel {
            timeTracker: TimeTracker
        }
    }

    TimeLogView {
        id: timeLogView

        anchors.fill: parent
        reverse: true
        lazyDialogs: false
        model: timeLogModel

        onInsert: timeLogModel.insertItem(modelIndex, newData)
        onAppend: timeLogModel.appendItem(newData)
        onRemove: timeLogModel.removeItem(modelIndex)
    }

    Loader {
        anchors.margins: 32
        anchors.fill: parent
        active: timeLogView.count === 0
        sourceComponent: Component {
            Label {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 34
                wrapMode: Text.Wrap
                color: view.Material.hintTextColor
                text: qsTr("Click on the \u201C+\u201D button to create a new record and start tracking your time")
            }
        }
    }

    FloatingActionButton {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: height / 2
        iconSource: "images/ic_add_white_24dp.png"

        onClicked: timeLogView.itemAppend()
    }
}
