import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3
import QtQuick.Extras 1.4
import "Util.js" as Util

Item {
    id: delegateEditor

    property date startTimeAfter
    property date startTimeBefore

    property string category
    property var startTime
    property string comment

    QtObject {
        id: d

        property int durationTime: (startTimeBefore - startTime) / 1000 |0
    }

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

                placeholderText: "Enter category"

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

                minimumDate: startTimeAfter
                maximumDate: startTimeBefore

                onClicked: {
                    var newDate = new Date(date)
                    delegateEditor.startTime.setFullYear(newDate.getFullYear())
                    delegateEditor.startTime.setMonth(newDate.getMonth())
                    delegateEditor.startTime.setDate(newDate.getDate())
                    delegateEditor.startTime = delegateEditor.startTime
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
                            delegateEditor.startTime = delegateEditor.startTime
                        }
                    }
                }
                TumblerColumn {
                    model: 60

                    onCurrentIndexChanged: {
                        if (delegateEditor.startTime !== undefined) {
                            delegateEditor.startTime.setMinutes(currentIndex)
                            delegateEditor.startTime = delegateEditor.startTime
                        }
                    }
                }
                TumblerColumn {
                    model: 60

                    onCurrentIndexChanged: {
                        if (delegateEditor.startTime !== undefined) {
                            delegateEditor.startTime.setSeconds(currentIndex)
                            delegateEditor.startTime = delegateEditor.startTime
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

        Label {
            width: parent.width
            elide: Text.ElideLeft
            text: "Duration: %1".arg(Util.durationText(d.durationTime))
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: "Comment (optional)"
            }

            TextField {
                property alias origComment: delegateEditor.comment

                placeholderText: "Optional comment"

                onTextChanged: delegateEditor.comment = text

                onOrigCommentChanged: text = origComment
            }
        }
    }
}
