import QtQuick
import QtQuick.Controls

Button {
    id: root
    property color accent: "#4F8CFF"
    height: 44
    font.pixelSize: 14
    font.weight: Font.DemiBold
    background: Rectangle {
        radius: 8
        color: root.enabled ? (root.down ? Qt.darker(root.accent, 1.25) : root.accent) : "#2A3140"
        border.color: root.hovered ? "#FFFFFF" : "transparent"
        border.width: root.hovered ? 1 : 0
    }
    contentItem: Text {
        text: root.text
        color: root.enabled ? "#FFFFFF" : "#7C8798"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font: root.font
        elide: Text.ElideRight
    }
}

