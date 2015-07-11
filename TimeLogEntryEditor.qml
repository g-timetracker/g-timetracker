import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3
import QtQuick.Extras 1.4

Item {
    id: delegateEditor

    property string category
    property var startTime
    property string durationTime
    property string comment

    implicitWidth: 600
    implicitHeight: itemsColumn.implicitHeight

    Column {
        id: itemsColumn

        width: parent.width
        spacing: 10

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Category:"
            }

            TextField {
                property alias origCategory: delegateEditor.category

                onTextChanged: delegateEditor.category = text

                onOrigCategoryChanged: text = origCategory
            }
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Start:"
            }

            Calendar {
                property alias origDate: delegateEditor.startTime

                onClicked: {
                    var newDate = new Date(date)
                    delegateEditor.startTime.setFullYear(newDate.getFullYear())
                    delegateEditor.startTime.setMonth(newDate.getMonth())
                    delegateEditor.startTime.setDate(newDate.getDate())
                }

                onOrigDateChanged: selectedDate = origDate
            }

            Tumbler {
                id: timeTumbler

                property alias origTime: delegateEditor.startTime

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

                onOrigTimeChanged: {
                    setCurrentIndexAt(0, origTime.getHours())
                    setCurrentIndexAt(1, origTime.getMinutes())
                    setCurrentIndexAt(2, origTime.getSeconds())
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
                property alias origComment: delegateEditor.comment

                onTextChanged: delegateEditor.comment = text

                onOrigCommentChanged: text = origComment
            }
        }
    }
}
