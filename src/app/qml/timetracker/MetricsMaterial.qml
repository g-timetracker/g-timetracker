pragma Singleton
import QtQml 2.2
import QtQuick.Controls 2.0

QtObject {
    property ApplicationWindow applicationWindow: null
    property bool isLandscape: !!applicationWindow
                               && applicationWindow.width > applicationWindow.height
}
