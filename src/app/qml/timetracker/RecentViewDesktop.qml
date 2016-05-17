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
import TimeLog 1.0

Item {
    property string title: qsTranslate("main window", "Recent")

    ReverseProxyModel {
        id: timeLogModel

        sourceModel: TimeLogRecentModel {
            timeTracker: TimeTracker
        }
    }

    ColumnLayout {
        anchors.fill: parent

        TimeLogView {
            id: timeLogView

            Layout.fillHeight: true
            Layout.fillWidth: true
            reverse: true
            model: timeLogModel

            onInsert: timeLogModel.insertItem(modelIndex, newData)
            onAppend: timeLogModel.appendItem(newData)
            onRemove: timeLogModel.removeItem(modelIndex)
        }

        Item {
            Layout.fillHeight: false
            Layout.fillWidth: true
            implicitHeight: 50

            PushButton {
                anchors.centerIn: parent
                text: qsTranslate("TimeLogView", "Add")
                onClicked: timeLogView.itemAppend()
            }
        }
    }
}
