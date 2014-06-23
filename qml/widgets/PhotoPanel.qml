import QtQuick 2.0
import QtMultimedia 5.0

import com.ics.faces 1.0

Item {
    id: container

    property int margin: 3
    property int padding: 3
    property int bottom_anchor: 40
    property int top_anchor: 0

    anchors.horizontalCenter: parent.horizontalCenter

    signal imageChanged(string name);

    x: margin
    y: margin + top_anchor

    width: parent.width - 2 * padding
    height: parent.height - 2 * padding - bottom_anchor - top_anchor

    function update_name(new_name) {
        imageProcessor.update_name(new_name);
    }

    function take_photo() {
        if (imageProcessor.can_capture())
            camera.imageCapture.capture();
        else {
            imageProcessor.set_random_image();
            imageProcessor.recognize();
        }
    }

    function set_photo(img_id) {
        console.log("Set photo: " + img_id);
        photoPreview.source = "image://processed_images/" + img_id;
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

    Camera {
        id: camera

        imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceFlash

        exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }

        flash.mode: Camera.FlashRedEyeReduction

        imageCapture {
            onImageCaptured: {
                set_photo(preview.split("/").slice(-1)[0]);
                imageProcessor.process_image(preview);
            }
        }
    }

    VideoOutput {
        source: camera
        anchors.fill: parent
        focus : visible // to receive focus and capture key events when visible

        height: parent.height - 2 * margin
        width: parent.width - 2 * margin
        fillMode: Image.PreserveAspectCrop
    }

    Image {
        id: photoPreview
        asynchronous: false

        anchors.fill: parent

        height: parent.height - 2 * margin
        width: parent.width - 2 * margin

        fillMode: Image.PreserveAspectCrop
    }

    ImageProcessor {
        id: imageProcessor
        onImageChanged: {
            set_photo(imageId);
            container.imageChanged(name);
        }
    }
}
