import QtQuick 2.0
import QtQuick.Window 2.1

import "widgets"

Window {
    property int text_size: 23
    property int padding: 5

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
        asynchronous: false
    }

    TrainingPicture {
        id: photo_panel
        bottom_anchor: buttons.height + padding
        padding: padding
    }

    BtnGroup {
        id: buttons
        height: parent.height / 10
        text_size: text_size
        margin: parent.height / 60
        padding: padding
        btn_1_text: "Next photo"
        btn_2_text: "Close"

        onBtn_1_pressed: {
            photo_panel.next_image()
        }
        onBtn_2_pressed: {
            Qt.quit()
        }
    }
}
