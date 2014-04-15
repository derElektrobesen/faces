import QtQuick 2.0
import QtMultimedia 5.0

Item {
    id: container

    property int margin: 3
    property int bottom_anchor: 40

    anchors.horizontalCenter: parent.horizontalCenter

    x: margin
    y: margin

    width: parent.width - 2 * margin
    height: parent.height - 3 * margin - bottom_anchor

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
                photoPreview.source = preview  // Show the preview in an Image
            }
        }
    }

    VideoOutput {
        source: camera
        anchors.fill: parent
        focus : visible // to receive focus and capture key events when visible
    }

    Image {
        id: photoPreview
        asynchronous: true

        anchors.fill: parent

        height: parent.height - 2 * margin
        width: parent.width - 2 * margin

        fillMode: Image.PreserveAspectCrop
    }
}
