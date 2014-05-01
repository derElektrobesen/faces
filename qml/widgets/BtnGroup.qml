import QtQuick 2.0

Item {
    id: container

    property int margin: 3
    property int padding: 3
    property string btn_1_text: "Button 1"
    property string btn_2_text: "Button 2"
    property int text_size: 20

    signal btn_1_pressed
    signal btn_2_pressed

    width: parent.width - 2 * padding
    height: 40

    anchors.horizontalCenter: parent.horizontalCenter

    x: padding
    y: parent.height - height - padding

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

    Button {
        id: btn_1

        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: margin

        width: parent.width / 2 - 2 * margin
        height: parent.height - 2 * margin
        x: margin

        text: btn_1_text
        text_size: text_size

        onClicked: {
            container.btn_1_pressed()
        }
    }

    Button {
        id: btn_2

        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 2 * margin
        anchors.rightMargin: margin
        anchors.left: btn_1.right

        width: parent.width / 2 - 2 * margin
        height: parent.height - 2 * margin

        text: btn_2_text
        text_size: text_size

        onClicked: {
            container.btn_2_pressed()
        }
    }
}
