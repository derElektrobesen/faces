import QtQuick 2.0

Item {
    id: container

    property int margin: 3
    property string btn_1_text: "Button 1"
    property string btn_2_text: "Button 2"

    width: parent.width - 6
    height: 40

    anchors.horizontalCenter: parent.horizontalCenter

    x: margin
    y: parent.height - height - margin

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
        x: margin

        text: btn_1_text
    }

    Button {
        id: btn_2

        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 2 * margin
        anchors.rightMargin: margin
        anchors.left: btn_1.right

        width: parent.width / 2 - 2 * margin

        text: btn_2_text
    }
}
