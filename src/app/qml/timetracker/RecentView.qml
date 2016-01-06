import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import TimeLog 1.0

Item {
    property string title: "Recent"

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

            Button {
                anchors.centerIn: parent
                action: timeLogView.appendAction
            }
        }
    }
}
