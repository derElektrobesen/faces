import QtQuick 2.0
import QtMultimedia 5.0

import com.ics.faces 1.0

Item {
    id: container

    property int margin: 3
    property int padding: 3
    property int bottom_anchor: 40

    anchors.horizontalCenter: parent.horizontalCenter

    x: margin
    y: margin

    width: parent.width - 2 * padding
    height: parent.height - 2 * padding - bottom_anchor

    function set_photo(img_id) {
        // console.log(img_id);
        photoPreview.source = img_id;
    }

    function next_image() {
        imageProcessor.next_image();
    }

    function process_rect(x1, y1, x2, y2) {
        var x_scale, y_scale, x_offset = 0, y_offset = 0;
        if (photoPreview.paintedWidth != photoPreview.width) {
            x_offset = (photoPreview.width - photoPreview.paintedWidth) / 2;
            x1 -= x_offset;
            x2 -= x_offset;
        } else if (photoPreview.paintedHeight != photoPreview.height) {
            y_offset = (photoPreview.height - photoPreview.paintedHeight) / 2;
            y1 -= y_offset;
            y2 -= y_offset;
        }

        x_scale = (photoPreview.width - 2 * x_offset) / photoPreview.sourceSize.width;
        y_scale = (photoPreview.height - 2 * y_offset) / photoPreview.sourceSize.height;

        imageProcessor.draw_rect(x1 / x_scale, y1 / y_scale, x2 / x_scale, y2 / y_scale);
    }

    Rectangle {
        width: container.width
        height: container.height

        opacity: 0.7
        color: "#343434"

        border.width: 1
        border.color: "#2a2c2f"

        radius: 3
    }

    Image {
        id: photoPreview
        asynchronous: true

        anchors.fill: parent

        height: parent.height - 2 * margin
        width: parent.width - 2 * margin

        fillMode: Image.PreserveAspectCrop
    }

    MouseArea {
        id: mouse_area
        anchors.fill: photoPreview

        property int x1: 0
        property int y1: 0

        onPressed: {
            x1 = mouse.x;
            y1 = mouse.y;
        }
        onReleased: {
            process_rect(x1, y1, mouse.x, mouse.y);
        }
    }

    TrainingPreview {
        id: imageProcessor
        onImageChanged: {
            set_photo(imageId);
        }
        onTextChanged: {
            cur_sign.text = text;
        }
    }

    Text {
        id: cur_sign
        color: "red"
        font.pointSize: 20
        x: 0
        y: 0
    }
}
