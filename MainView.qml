import QtQuick 2.0
import QtQuick.Layouts 1.3
import Qt.labs.controls 1.0

Item {
    id: mainView

    function showSearch() {
        if (d.searchIndex === -1) {
            d.searchIndex = tabModel.count
            tabModel.append({ "text": "Search", "source": "SearchView.qml",
                                "hasCloseButton": true })
        }
        tabBar.setCurrentIndex(d.searchIndex)
    }

    QtObject {
        id: d

        property int searchIndex: -1
    }

    ListModel {
        id: tabModel

        ListElement {
            text: "Recent entries"
            source: "RecentView.qml"
            hasCloseButton: false
        }
    }

    ColumnLayout {
        anchors.fill: parent

        TabBar {
            id: tabBar

            Layout.fillHeight: false
            Layout.fillWidth: true
            visible: count > 1

            Repeater {
                model: tabModel
                delegate: TabButton {
                    text: model.text

                    ToolButton {
                        anchors.right: parent.right
                        visible: model.hasCloseButton
                        text: "x"
                        onClicked: tabModel.remove(model.index)
                    }
                }
            }
        }

        StackLayout {
            id: stackLayout

            Layout.fillHeight: true
            Layout.fillWidth: true
            currentIndex: tabBar.currentIndex

            Repeater {
                model: tabModel
                delegate: Loader {
                    source: model.source
                }
            }
        }
    }
}
