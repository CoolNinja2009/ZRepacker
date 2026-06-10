import QtQuick

Rectangle {
    id: root
    property string label: ""
    property string value: ""
    radius: 8
    color: "#151922"
    border.color: "#252D3B"
    height: 70
    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6
        Text { text: root.label; color: "#9AA4B2"; font.pixelSize: 11 }
        Text { text: root.value; color: "#F4F7FB"; font.pixelSize: 17; font.weight: Font.DemiBold; width: parent.width; elide: Text.ElideRight }
    }
}

