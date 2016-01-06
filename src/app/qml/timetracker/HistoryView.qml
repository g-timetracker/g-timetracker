import QtQuick 2.4
import QtQuick.Layouts 1.1
import TimeLog 1.0

Item {
    property string title: "History"
    property alias beginDate: timeLogFilter.beginDate
    property alias endDate: timeLogFilter.endDate

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
