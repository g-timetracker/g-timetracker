import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQml.Models 2.2
import TimeLog 1.0

Item {
    property alias category: timeLogFilter.category

    TimeLogSearchModel {
        id: timeLogModel

        begin: timeLogFilter.beginDate
        end: timeLogFilter.endDate
        category: timeLogFilter.category
    }

    DelegateModel {
        id: delegateModel

        model: timeLogModel
        delegate: TimeLogDelegate {
            width: listView.width
            category: model.category
            startTime: model.startTime
            durationTime: model.durationTime
            comment: model.comment
            precedingStart: model.precedingStart
            succeedingStart: model.succeedingStart
        }
    }

    TimeLogEditDialog {
        id: editDialog
    }

    ColumnLayout {
        anchors.fill: parent

        TimeLogFilter {
            id: timeLogFilter

            Layout.fillHeight: false
            Layout.fillWidth: true
        }

        ListView {
            id: listView

            Layout.fillHeight: true
            Layout.fillWidth: true
            verticalLayoutDirection: ListView.TopToBottom
            clip: true
            model: delegateModel

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                acceptedButtons: Qt.LeftButton

                onDoubleClicked: {
                    var item = listView.itemAt(mouse.x + listView.contentX,
                                               mouse.y + listView.contentY)
                    if (item) {
                        editDialog.openDialog(item)
                    }
                }
            }
        }
    }
}
