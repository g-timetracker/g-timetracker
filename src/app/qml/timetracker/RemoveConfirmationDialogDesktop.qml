import QtQuick.Dialogs 1.2

MessageDialog {
    title: qsTranslate("main window", "Remove confirmation")
    icon: StandardIcon.Question
    standardButtons: StandardButton.Yes | StandardButton.No
    onYes: accept()
}
