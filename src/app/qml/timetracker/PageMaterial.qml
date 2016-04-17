import QtQuick 2.5
import Qt.labs.controls 1.0

Page {
    property alias source: loader.source
    property Component toolBar
    property string title: loader.item && loader.item.title ? loader.item.title : ""
    property alias content: loader.item

    header: Loader {
        sourceComponent: toolBar
    }

    Loader {
        id: loader

        anchors.fill: parent
    }
}
