import QtQuick 2.0

Rectangle {
    anchors.fill: parent

    property color color_top: "#ADAFB3"
    property color color_bot: "#3B3B3B"

    border.width: 1
    border.color: "#616261"

    smooth: true

    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: color_top
        }
        GradientStop {
            position: 1
            color: color_bot
        }
    }
}
