import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import TimeLog 1.0

Page {
    id: view

    property alias beginDate: timeLogFilter.beginDate
    property alias endDate: timeLogFilter.endDate

    title: qsTranslate("main window", "History")

    header: MainToolBarMaterial {
        title: view.title
        isBottomItem: view.StackView.index === 0
    }

    TimeLogSearchModel {
        id: timeLogModel

        timeTracker: TimeTracker
        begin: timeLogFilter.beginDate
        end: timeLogFilter.endDate
    }

    ColumnLayout {
        anchors.fill: parent

        DatePeriodPicker {
            id: timeLogFilter

            Layout.fillHeight: false
            Layout.fillWidth: true
        }

        TimeLogView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            model: timeLogModel

            onInsert: timeLogModel.insertItem(modelIndex, newData)
            onAppend: timeLogModel.appendItem(newData)
            onRemove: timeLogModel.removeItem(modelIndex)
        }
    }
}
