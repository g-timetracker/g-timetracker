import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import TimeLog 1.0

Repeater {
    id: repeater

    property alias category: categoryModel.category

    model: TimeLogCategoryDepthModel {
        id: categoryModel

        timeTracker: TimeTracker
    }

    RowLayout {
        Layout.columnSpan: 2
        spacing: 10

        Label {
            Layout.fillWidth: true
            text: "%1Category:".arg(new Array(index+1).join("Sub"))
        }

        ComboBox {
            id: combobox

            property int subcategory: subcategoryIndex === undefined ? -1 : subcategoryIndex

            currentIndex: subcategory
            model: subcategories ? [ "" ].concat(subcategories) : []

            onCurrentIndexChanged: {
                if (subcategory != currentIndex) {
                    subcategoryIndex = currentIndex
                }
            }

            onSubcategoryChanged: {
                if (currentIndex != subcategory) {
                    currentIndex = subcategory
                }
            }
        }
    }
}
