import QtQuick 2.0

Item {
    id: container

    property string text: "Button text"
    property int text_size: 15

    signal clicked

    width: 80
    height: 30

    BtnGradient {
        id: btn
        color_top: "#7d7e7d"
        color_bot: "#0e0e0e"
        radius: 10
    }

    BtnGradient {
        id: pressed
        opacity: 0
        color_top: "#646464"
        color_bot: "#282828"
        radius: 10
    }

    Text {
        id: btn_text
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -1

        font.pixelSize: text_size
        style: Text.Outline
        styleColor: "#616261"
        color: "white"
        smooth: false

        text: container.text
    }

    MouseArea {
        id: mouse_area
        anchors.fill: parent
        onClicked: {
            container.clicked()
        }
    }

    states: [
        State {
            name: "Pressed"
            when: mouse_area.pressed === true
            PropertyChanges {
                target: pressed
                opacity: 1
            }
        }
    ]
}
