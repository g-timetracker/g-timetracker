import QtQuick.Dialogs 1.2
import TimeLog 1.0

FileDialog {
    title: qsTranslate("settings", "Select sync folder")
    selectFolder: true

    onAccepted: Settings.syncPath = folder
}
