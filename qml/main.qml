import QtQuick 2.0
import QtQuick.Window 2.1

import "widgets"
import com.ics.faces 1.0

Window {
    id: container

    property int text_size: 23
    property int padding: 5

    property bool first_run: true

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

    PhotoPanel {
        id: photo_panel
        bottom_anchor: buttons.height + padding
        top_anchor: 0
        padding: padding

        onImageChanged: {
            img_info.set_image_info(name);
        }

        onImageRecognized: {
            name_viewer.text = name;
            name_viewer_wrapper.width = name_viewer.width + 2 * padding;
            name_viewer_wrapper.visible = true
            name_viewer.visible = true
        }
    }

    ImageInfoPanel {
        id: img_info
        height: parent.height / 10
        text_size: text_size
        margin: parent.height / 60
        padding: padding
        visible: false
        y: padding

        onFailBtnPressed: {
            img_info.visible = false;
            human_name_edt.visible = true;
            human_name_edt.init("Input name")
        }
    }

    InputBox {
        id: human_name_edt
        height: parent.height / 10
        text_size: text_size
        margin: parent.height / 60
        padding: padding
        y: padding

        visible: false

        onBtn_ok_pressed: {
            photo_panel.update_name(text)
            human_name_edt.visible = false
            img_info.visible = true
            img_info.set_image_info(text)
        }
    }

    BtnGroup {
        id: buttons
        height: parent.height / 10
        text_size: text_size
        margin: parent.height / 60
        padding: padding
        btn_1_text: "Take photo"
        btn_2_text: "Close"

        onBtn_1_pressed: {
            if (container.first_run) {
                container.first_run = false
                photo_panel.top_anchor = img_info.height + padding
                img_info.visible = true
            }

            photo_panel.take_photo()
        }
        onBtn_2_pressed: {
            Qt.quit()
        }
    }

    Rectangle {
        id: name_viewer_wrapper

        anchors.horizontalCenter: photo_panel.horizontalCenter
        y: photo_panel.y + photo_panel.height - text_size - padding
        height: text_size + 4
        visible: false

        smooth: true
        opacity: 0.7
        radius: 3

        border.width: 1
        border.color: "#2a2c2f"
        color: "black"
    }

    Text {
        id: name_viewer
        anchors.horizontalCenter: name_viewer_wrapper.horizontalCenter
        anchors.verticalCenter: name_viewer_wrapper.verticalCenter

        font.pixelSize: text_size
        visible: false
        opacity: 0.9

        color: "yellow"
    }
}
