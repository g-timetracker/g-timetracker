import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Item {
    id: mainView

    property bool isConfirmationsEnabled

    ColumnLayout {
        anchors.fill: parent

        TabBar {
            id: tabBar

            Layout.fillHeight: false
            Layout.fillWidth: true
            visible: count > 1

            TabButton {
                text: "Recent entries"
            }
        }

        StackLayout {
            id: stackLayout

            Layout.fillHeight: true
            Layout.fillWidth: true
            currentIndex: tabBar.currentIndex

            RecentView {
                isConfirmationsEnabled: mainView.isConfirmationsEnabled
            }
        }
    }
}
