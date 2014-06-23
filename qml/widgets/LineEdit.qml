import QtQuick 2.0

Item {
    property int margin: 3
    property int text_size: 20

    x: margin
    y: margin
    width: 50
    height: 20

    function get_text() {
        return text_edit.text;
    }

    function init(text) {
        text_edit.text = text
        text_edit.selectAll()
        text_edit.focus = true
    }

    Rectangle {
        x: parent.x
        y: parent.y
        width: parent.width
        height: parent.height
        color: "black"
        opacity: 0.8
        border.color: "#2a2c2f"
        border.width: 1

        radius: 3
    }

    TextInput {
        id: text_edit

        x: parent.x + margin
        y: parent.y + margin
        width: parent.width

        maximumLength: 255
        font.pixelSize: text_size

        color: "white"

        anchors.verticalCenter: parent.verticalCenter
    }
}
