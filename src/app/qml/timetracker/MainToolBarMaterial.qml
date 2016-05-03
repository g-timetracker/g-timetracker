import QtQuick.Controls 2.0
import TimeLog 1.0

ToolBarMaterial {
    property bool isBottomItem: true

    leftIcon: isBottomItem ? "images/ic_menu_white_24dp.png" : "images/ic_arrow_back_white_24dp.png"

    onLeftActivated: {
        if (isBottomItem) {
            TimeTracker.openNavigationDrawerRequested()
        } else {
            TimeTracker.backRequested()
        }
    }
}
