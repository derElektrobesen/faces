import QtQuick 2.0
import QtQuick.Window 2.1

import "widgets"

Window {
    id: main_window
    visible: true
    width: 300
    height: 500

    color: "#343434"

    Image {
        source: "images/stripes.png"
        fillMode: Image.Tile
        anchors.fill: parent
        opacity: 0.3
    }

    PhotoPanel {
        id: photo_panel
    }

    BtnGroup {
        id: buttons
    }
}
