import QtQuick 2.0
import QtQuick.Controls 1.4
import TimeLog 1.0

Item {
    TableView {
        anchors.fill: parent
        model: TimeLog.categories()
        headerVisible: false

        TableViewColumn { }
    }
}
