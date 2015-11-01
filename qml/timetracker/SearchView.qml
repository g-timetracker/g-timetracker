import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import TimeLog 1.0

Item {
    property alias category: timeLogFilter.category

    TimeLogSearchModel {
        id: timeLogModel

        begin: timeLogFilter.beginDate
        end: timeLogFilter.endDate
        category: timeLogFilter.category
    }

    ColumnLayout {
        anchors.fill: parent

        TimeLogFilter {
            id: timeLogFilter

            Layout.fillHeight: false
            Layout.fillWidth: true
        }

        TimeLogView {
            id: timeLogView

            Layout.fillHeight: true
            Layout.fillWidth: true
            model: timeLogModel
            menu: Menu {
                MenuItem {
                    action: timeLogView.editAction
                }
            }
        }
    }
}
