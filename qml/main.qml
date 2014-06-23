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
}
