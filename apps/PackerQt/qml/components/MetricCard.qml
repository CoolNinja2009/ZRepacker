import QtQuick

Rectangle {
    id: root
    property string label: ""
    property string value: ""
    property color accent: "#4F8CFF"
    radius: 8
    color: "#151922"
    border.color: "#252D3B"
    border.width: 1
    implicitHeight: 96

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10
        Text {
            text: root.label
            color: "#9AA4B2"
            font.pixelSize: 12
        }
        Text {
            text: root.value
            color: "#F4F7FB"
            font.pixelSize: 22
            font.weight: Font.DemiBold
            elide: Text.ElideRight
            width: parent.width
        }
    }

    Rectangle {
        width: 3
        radius: 2
        color: root.accent
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }
}

