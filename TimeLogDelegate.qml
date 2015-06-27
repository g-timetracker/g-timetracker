import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

Column {
    id: timeLogDelegate

    property string category
    property string startTime
    property string durationTime
    property string comment

    signal updateCategory(string newCategory)

    width: 400
    spacing: 10

    Loader {
        id: categoryLoader

        sourceComponent: labelComponent

        Component {
            id: labelComponent

            Row {
                spacing: 10

                Label {
                    text: timeLogDelegate.category
                }

                ToolButton {
                    iconName: "accessories-text-editor"

                    onClicked: categoryLoader.sourceComponent = editorComponent
                }
            }
        }

        Component {
            id: editorComponent

            TextField {
                text: timeLogDelegate.category

                onEditingFinished: {
                    timeLogDelegate.updateCategory(text)
                    categoryLoader.sourceComponent = labelComponent
                }
            }
        }
    }

    RowLayout {
        width: parent.width
        spacing: 10

        Label {
            id: startLabel

            text: "Start:"
        }

        Label {
            id: startDateLabel

            text: Qt.formatDate(timeLogDelegate.startTime)
        }

        Label {
            id: startTimeLabel

            text: Qt.formatTime(timeLogDelegate.startTime)
        }

        Label {
            id: durationTimeLabel

            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: "Duration: %1".arg(timeLogDelegate.durationTime)
        }
    }

    Label {
        id: commentLabel

        text: "Comment: %1".arg(timeLogDelegate.comment)
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        visible: !!text
    }
}

