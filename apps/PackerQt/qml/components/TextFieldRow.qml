import QtQuick
import QtQuick.Controls

Column {
    id: root
    property string label: ""
    property alias text: field.text
    property string placeholder: ""
    spacing: 7

    Text {
        text: root.label
        color: "#9AA4B2"
        font.pixelSize: 12
    }

    TextField {
        id: field
        width: parent.width
        height: 42
        placeholderText: root.placeholder
        color: "#F4F7FB"
        placeholderTextColor: "#667085"
        selectionColor: "#4F8CFF"
        selectedTextColor: "white"
        font.pixelSize: 14
        background: Rectangle {
            radius: 8
            color: "#10141D"
            border.color: field.activeFocus ? "#4F8CFF" : "#2A3140"
            border.width: 1
        }
    }
}

