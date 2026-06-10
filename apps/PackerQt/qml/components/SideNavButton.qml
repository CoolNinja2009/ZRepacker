import QtQuick
import QtQuick.Controls

Button {
    id: root
    property bool selected: false
    height: 42
    flat: true
    font.pixelSize: 14
    font.weight: selected ? Font.DemiBold : Font.Normal

    background: Rectangle {
        radius: 8
        color: root.selected ? "#20283A" : (root.hovered ? "#181E2A" : "transparent")
        border.color: root.selected ? "#344058" : "transparent"
    }

    contentItem: Text {
        text: root.text
        color: root.selected ? "#F4F7FB" : "#9AA4B2"
        verticalAlignment: Text.AlignVCenter
        leftPadding: 14
        font: root.font
    }
}

