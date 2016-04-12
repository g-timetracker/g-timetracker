import QtQuick 2.4
import TimeLog 1.0

Item {
    property string title: qsTranslate("main window", "Recent")

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
        model: timeLogModel

        onInsert: timeLogModel.insertItem(modelIndex, newData)
        onAppend: timeLogModel.appendItem(newData)
        onRemove: timeLogModel.removeItem(modelIndex)
    }

    FloatingActionButton {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: height / 2
        iconSource: "images/ic_add_white_24dp.png"

        onClicked: timeLogView.itemAppend()
    }
}
