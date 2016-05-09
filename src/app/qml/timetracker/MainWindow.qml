import Qt.labs.settings 1.0

MainWindowMaterial {
    id: window

    minimumWidth: 600
    minimumHeight: 360

    Settings {
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height

        category: "window"
    }
}
