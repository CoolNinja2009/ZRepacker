import QtQuick
import QtQuick.Controls

Button {
    id: root
    property bool selected: false
    property color accent: "#4F8CFF"
    width: 150
    height: 48
    font.pixelSize: 13
    font.weight: selected ? Font.DemiBold : Font.Normal
    background: Rectangle {
        radius: 8
        color: selected ? "#20283A" : "#10141D"
        border.color: selected ? root.accent : "#2A3140"
        border.width: 1
    }
    contentItem: Text {
        text: root.text
        color: selected ? "#F4F7FB" : "#9AA4B2"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font: root.font
    }
}

