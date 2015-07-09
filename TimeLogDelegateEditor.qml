import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3
import QtQuick.Extras 1.4
import QtQuick.Dialogs 1.2

Dialog {
    id: delegateEditor

    property TimeLogDelegate delegateItem
    property string category
    property var startTime
    property string durationTime
    property string comment

    width: 600
    standardButtons: StandardButton.Ok | StandardButton.Cancel

    Column {
//            width: 600    // FIXME
        width: parent.width
        spacing: 10

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Category:"
            }

            TextField {
                text: delegateEditor.category

                onTextChanged: {
                    delegateEditor.category = text
                }
            }
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Start:"
            }

            Calendar {
                selectedDate: delegateEditor.startTime

                onClicked: {
                    var newDate = new Date(date)
                    delegateEditor.startTime.setYear(newDate.getYear())
                    delegateEditor.startTime.setMonth(newDate.getMonth())
                    delegateEditor.startTime.setDate(newDate.getDate())
                }
            }

            Tumbler {
                id: timeTumbler

                property alias time: delegateEditor.startTime

                TumblerColumn {
                    model: 24

                    onCurrentIndexChanged: {
                        if (delegateEditor.startTime !== undefined) {
                            delegateEditor.startTime.setHours(currentIndex)
                        }
                    }
                }
                TumblerColumn {
                    model: 60

                    onCurrentIndexChanged: {
                        if (delegateEditor.startTime !== undefined) {
                            delegateEditor.startTime.setMinutes(currentIndex)
                        }
                    }
                }
                TumblerColumn {
                    model: 60

                    onCurrentIndexChanged: {
                        if (delegateEditor.startTime !== undefined) {
                            delegateEditor.startTime.setSeconds(currentIndex)
                        }
                    }
                }

                onTimeChanged: {
                    setCurrentIndexAt(0, time.getHours())
                    setCurrentIndexAt(1, time.getMinutes())
                    setCurrentIndexAt(2, time.getSeconds())
                }

            }
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Comment (optional)"
            }

            TextField {
                text: delegateEditor.comment

                onTextChanged: {
                    delegateEditor.comment = text
                }
            }
        }
    }

    onDelegateItemChanged: {
        category = delegateItem.category
        startTime = new Date(delegateItem.startTime)
        comment = delegateItem.comment
    }

    onAccepted: {
        delegateItem.updateData(category, startTime, comment)
    }
}
