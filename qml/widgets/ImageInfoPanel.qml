import QtQuick 2.0

Item {
    id: container

    property int margin: 3
    property int padding: 3
    property string btn_1_text: "Button 1"
    property string btn_2_text: "Button 2"
    property int text_size: 20

    signal failBtnPressed

    width: parent.width - 2 * padding
    height: 40

    anchors.horizontalCenter: parent.horizontalCenter

    x: padding
    y: parent.height - height - padding

    function set_image_info(name) {
        text.text = name
    }

    Rectangle {
        width: parent.width
        height: parent.height

        smooth: true
        opacity: 0.4
        radius: 3

        border.width: 1
        border.color: "#2a2c2f"
        color: "black"
    }

    Text {
        id: text
        width: 3 * parent.width / 5
        font.pixelSize: text_size
        anchors.verticalCenter: parent.verticalCenter
        x: margin

        color: "white"
    }

    Button {
        id: fail_btn

        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 2 * margin
        anchors.rightMargin: margin

        x: text.width + margin * 2
        width: parent.width - x - margin
        height: parent.height - 2 * margin

        text: "Recognize fail"
        text_size: 13

        onClicked: {
            container.failBtnPressed()
        }

    }
}
