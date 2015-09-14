import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Item {
    id: mainView

    ListModel {
        id: tabModel

        ListElement {
            text: "Recent entries"
            source: "RecentView.qml"
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
