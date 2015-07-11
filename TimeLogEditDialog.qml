import QtQuick 2.4
import QtQuick.Dialogs 1.2

Dialog {
    id: editDialog

    property TimeLogDelegate delegateItem

    width: delegateEditor.implicitWidth
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    title: "Edit entry"

    TimeLogEntryEditor {
        id: delegateEditor
    }

    onDelegateItemChanged: {
        delegateEditor.category = delegateItem.category
        delegateEditor.startTime = new Date(delegateItem.startTime)
        delegateEditor.durationTime = 0
        delegateEditor.comment = delegateItem.comment
    }

    onAccepted: {
        delegateItem.updateData(delegateEditor.category, delegateEditor.startTime,
                                delegateEditor.comment)
    }
}
