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
        console.log("Set image: " + img_id)
        photoPreview.source = img_id;
    }

    function next_image() {
        imageProcessor.next_image();
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

    TrainingPreview {
        id: imageProcessor
        onImageChanged: {
            console.log("HEllo");
            set_photo(imageId);
        }
    }
}
