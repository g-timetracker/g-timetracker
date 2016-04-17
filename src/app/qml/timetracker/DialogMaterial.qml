import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import Qt.labs.controls 1.0
import Qt.labs.controls.material 1.0

Popup {
    id: dialog

    default property alias dialogContent: contentContainer.children
    property string title: ""
    property bool customTitleBar: false
    property string affirmativeText: qsTranslate("dialog", "OK")
    property string dismissiveText: qsTranslate("dialog", "Cancel")
    property point position: ApplicationWindow.contentItem.mapToItem(parent,
                                                                     (ApplicationWindow.window.width - width) / 2,
                                                                     (ApplicationWindow.window.height - height) / 2)

    function accept() {
        accepted()
        close()
    }

    function reject() {
        rejected()
        close()
    }

    signal accepted()
    signal rejected()

    closePolicy: Popup.OnEscape | Popup.OnReleaseOutside
    modal: true
    padding: 0
    margins: 16
    x: position.x
    y: position.y

    contentItem: ColumnLayout {
        spacing: 0

        Item {
            Layout.fillHeight: false
            Layout.fillWidth: true
            implicitHeight: 24
            visible: !dialog.customTitleBar
        }

        Item {
            Layout.fillHeight: false
            Layout.fillWidth: true
            implicitHeight: titleLabel.implicitHeight + titleLabel.anchors.bottomMargin
            implicitWidth: titleLabel.implicitWidth + titleLabel.anchors.leftMargin + titleLabel.anchors.rigthMargin
            visible: !!dialog.title

            Label {
                id: titleLabel

                anchors.leftMargin: 24
                anchors.rightMargin: 24
                anchors.bottomMargin: 20
                anchors.fill: parent
                wrapMode: Text.Wrap
                font.pixelSize: 20
                font.weight: Font.Medium
                color: dialog.Material.primaryTextColor
                text: dialog.title
            }
        }

        GridLayout {
            id: contentContainer

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft
        }

        DialogMaterialButtonBox {
            id: buttonBox

            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.minimumWidth: implicitWidth
            affirmativeText: dialog.affirmativeText
            dismissiveText: dialog.dismissiveText

            onAccepted: dialog.accept()
            onRejected: dialog.reject()
        }
    }
}
