import QtQuick.Dialogs 1.2

MessageDialog {
    signal closed()

    title: qsTranslate("main window", "Remove confirmation")
    icon: StandardIcon.Question
    standardButtons: StandardButton.Yes | StandardButton.No
    onYes: accept()
    onAccepted: closed()
    onRejected: closed()
}
