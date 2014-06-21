import QtQuick 2.0

Item {
    id: container

    property int margin: 3
    property int padding: 3
    property int text_size: 20

    width: parent.width - 2 * padding
    height: 40

    signal btn_ok_pressed(string text)

    x: padding
    y: parent.height - height - padding

    function init(text) {
        text_edit.init(text)
    }

    Rectangle {
        id: background

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
        id: btn_ok

        x: background.width - width - margin
        y: margin

        width: parent.height + 2 * margin
        height: parent.height - 2 * margin

        text: "Ok"
        text_size: text_size

        onClicked: {
            container.btn_ok_pressed(text_edit.get_text())
        }
    }

    LineEdit {
        id: text_edit
        width: parent.width - btn_ok.width - 6 * margin
        height: parent.height - 4 * margin

        text_size: text_size
    }
}
