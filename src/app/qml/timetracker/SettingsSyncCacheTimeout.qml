import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0
import TimeLog 1.0

Page {
    id: settingsDialog

    property string title: "Sync cache timeout"

    function close() {
        TimeTracker.backRequested()
    }

    visible: false

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                text: "back"
                label: Image {
                    anchors.centerIn: parent
                    source: "images/ic_arrow_back_white_24dp.png"
                }

                onClicked: settingsDialog.close()
            }

            LabelControl {
                Layout.fillWidth: true
                Material.theme: Material.Dark
                font.pixelSize: 20
                text: title
            }
        }
    }

    Flickable {
        anchors.bottomMargin: Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio
        anchors.fill: parent
        contentWidth: settingsItem.width
        contentHeight: settingsItem.height
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { }

        Item {
            id: settingsItem

            width: settingsDialog.width
            implicitHeight: container.height + container.anchors.margins * 2

            Column {
                id: container

                anchors.margins: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 16

                SpinBox {
                    anchors.horizontalCenter: parent.horizontalCenter
                    to: 24 * 3600
                    value: Settings.syncCacheTimeout
                    onValueChanged: Settings.syncCacheTimeout = value
                }

                Label {
                    width: parent.width
                    color: Material.hintTextColor
                    wrapMode: Text.Wrap
                    text: "With this setting you can control, how much time (in seconds) application would wait after last record before sync.\n" +
                          "You can set it to '0' to never sync by timeout."
                }
            }
        }
    }
}
