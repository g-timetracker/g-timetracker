import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: settingsDialog

    function close() {
        TimeTracker.backRequested()
    }

    title: qsTranslate("settings", "Sync cache size")
    visible: false

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                text: "back"
                contentItem: Image {
                    fillMode: Image.Pad
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
                    editable: true
                    value: Settings.syncCacheSize
                    onValueChanged: Settings.syncCacheSize = value
                }

                Label {
                    width: parent.width
                    color: Material.hintTextColor
                    wrapMode: Text.Wrap
                    text: qsTranslate("settings", "With this setting you can control, after which"
                                      + " amount of records application start the synchronization.\n\n"
                                      + "To make it sync after each record, set it to \u20180\u2019.")
                }
            }
        }
    }
}
