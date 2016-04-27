import QtQuick 2.5
import QtQuick.Controls 2.0

Page {
    property alias source: loader.source
    property Component toolBar
    property alias content: loader.item

    title: loader.item && loader.item.title ? loader.item.title : ""

    header: Loader {
        sourceComponent: toolBar
    }

    Loader {
        id: loader

        anchors.fill: parent
    }
}
