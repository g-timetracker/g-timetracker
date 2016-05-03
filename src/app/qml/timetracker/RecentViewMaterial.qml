import QtQuick 2.6
import QtQuick.Controls 2.0
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
