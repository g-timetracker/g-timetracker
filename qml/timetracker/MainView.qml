import QtQuick 2.0
import QtQuick.Layouts 1.3
import Qt.labs.controls 1.0

Item {
    id: mainView

    function showSearch(category) {
        if (d.searchIndex === -1) {
            d.searchIndex = tabModel.count
            tabModel.append({ "text": "Search", "source": "SearchView.qml",
                                "hasCloseButton": true })
        }
        tabBar.setCurrentIndex(d.searchIndex)
        if (category !== undefined) {
            stackLayout.children[d.searchIndex].item.category = category
        }
    }

    function showStats(category) {
        if (d.statsIndex === -1) {
            d.statsIndex = tabModel.count
            tabModel.append({ "text": "Statistics", "source": "StatsView.qml",
                                "hasCloseButton": true })
        }
        tabBar.setCurrentIndex(d.statsIndex)
        if (category !== undefined) {
            stackLayout.children[d.statsIndex].item.category = category
        }
    }

    function showCategories() {
        if (d.categoriesIndex === -1) {
            d.categoriesIndex = tabModel.count
            tabModel.append({ "text": "Categories", "source": "CategoriesView.qml",
                                "hasCloseButton": true })
            stackLayout.children[d.categoriesIndex].item.mainView = mainView
        }
        tabBar.setCurrentIndex(d.categoriesIndex)
    }

    QtObject {
        id: d

        property int searchIndex: -1
        property int statsIndex: -1
        property int categoriesIndex: -1
    }

    ListModel {
        id: tabModel

        ListElement {
            text: "Recent entries"
            source: "RecentView.qml"
            hasCloseButton: false
        }
    }

    ColumnLayout {
        anchors.fill: parent

        TabBar {
            id: tabBar

            Layout.fillHeight: false
            Layout.fillWidth: true
            visible: count > 1

            Repeater {
                model: tabModel
                delegate: TabButton {
                    text: model.text

                    ToolButton {
                        anchors.right: parent.right
                        visible: model.hasCloseButton
                        text: "x"
                        onClicked: tabModel.remove(model.index)
                    }
                }
            }
        }

        StackLayout {
            id: stackLayout

            Layout.fillHeight: true
            Layout.fillWidth: true
            currentIndex: tabBar.currentIndex

            Repeater {
                model: tabModel
                delegate: Loader {
                    source: model.source
                }
            }
        }
    }
}
