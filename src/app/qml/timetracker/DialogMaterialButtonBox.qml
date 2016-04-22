import QtQuick 2.5
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0

Item {
    id: buttonBox

    property string affirmativeText: qsTranslate("dialog", "OK")
    property string dismissiveText: qsTranslate("dialog", "Cancel")

    signal accepted()
    signal rejected()

    implicitWidth: 56 * 5
    implicitHeight: 52

    RowLayout {
        anchors.leftMargin: 24
        anchors.rightMargin: 8
        anchors.fill: parent
        spacing: 8

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Button {
            Layout.fillWidth: false
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignVCenter
            text: buttonBox.dismissiveText
            visible: !!text
            onClicked: buttonBox.rejected()
        }

        Button {
            Layout.fillWidth: false
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignVCenter
            text: buttonBox.affirmativeText
            visible: !!text
            onClicked: buttonBox.accepted()
        }
    }
}
