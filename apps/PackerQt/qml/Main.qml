import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import RepackStudio
import "components"

ApplicationWindow {
    id: window
    width: 1180
    height: 760
    minimumWidth: 980
    minimumHeight: 640
    visible: true
    title: "Repack Studio"
    color: "#0B0D10"

    PackerController { id: packer }

    property int page: 0
    property color accent: packer.accentColor

    FolderDialog {
        id: sourceDialog
        title: "Select source folder"
        onAccepted: packer.sourceFolder = selectedFolder.toString().replace("file:///", "").replace(/\//g, "\\")
    }
    FolderDialog {
        id: outputDialog
        title: "Select output folder"
        onAccepted: packer.outputFolder = selectedFolder.toString().replace("file:///", "").replace(/\//g, "\\")
    }

    Rectangle {
        anchors.fill: parent
        color: "#0B0D10"

        Rectangle {
            id: sidebar
            width: 220
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            color: "#0F131B"
            border.color: "#202838"

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 12

                Text {
                    text: "Repack Studio"
                    color: "#F4F7FB"
                    font.pixelSize: 22
                    font.weight: Font.Bold
                    height: 48
                    verticalAlignment: Text.AlignVCenter
                }

                SideNavButton { width: parent.width; text: "Dashboard"; selected: page === 0; onClicked: page = 0 }
                SideNavButton { width: parent.width; text: "Compression"; selected: page === 1; onClicked: page = 1 }
                SideNavButton { width: parent.width; text: "Branding"; selected: page === 2; onClicked: page = 2 }
                SideNavButton { width: parent.width; text: "Build"; selected: page === 3; onClicked: page = 3 }

                Item { height: 20; width: 1 }

                Rectangle {
                    width: parent.width
                    height: 86
                    radius: 8
                    color: "#141A25"
                    border.color: "#253046"
                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8
                        Text { text: "Status"; color: "#9AA4B2"; font.pixelSize: 12 }
                        Text {
                            text: packer.building ? "Building" : "Ready"
                            color: packer.building ? accent : "#4ADE80"
                            font.pixelSize: 18
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }
        }

        Item {
            anchors.left: sidebar.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            Column {
                anchors.fill: parent
                anchors.margins: 28
                spacing: 22

                Row {
                    width: parent.width
                    height: 52
                    Text {
                        text: page === 0 ? "Project Dashboard" : page === 1 ? "Compression Settings" : page === 2 ? "Installer Customization" : "Build Process"
                        color: "#F4F7FB"
                        font.pixelSize: 30
                        font.weight: Font.Bold
                        verticalAlignment: Text.AlignVCenter
                        width: parent.width - 160
                    }
                    PrimaryButton {
                        width: 150
                        text: packer.building ? "Building" : "Start Build"
                        enabled: !packer.building
                        accent: window.accent
                        onClicked: packer.startBuild()
                    }
                }

                StackLayout {
                    width: parent.width
                    height: parent.height - 74
                    currentIndex: page

                    Item {
                        Grid {
                            id: metrics
                            columns: 3
                            spacing: 14
                            width: parent.width
                            MetricCard { width: (metrics.width - 28) / 3; label: "Game"; value: packer.gameName; accent: window.accent }
                            MetricCard { width: (metrics.width - 28) / 3; label: "Version"; value: packer.version; accent: window.accent }
                            MetricCard { width: (metrics.width - 28) / 3; label: "Estimate"; value: packer.estimatedSize; accent: window.accent }
                        }

                        Rectangle {
                            anchors.top: metrics.bottom
                            anchors.topMargin: 18
                            width: parent.width
                            height: 330
                            radius: 8
                            color: "#151922"
                            border.color: "#252D3B"

                            Column {
                                anchors.fill: parent
                                anchors.margins: 18
                                spacing: 14

                                TextFieldRow { width: parent.width; label: "Game name"; text: packer.gameName; onTextChanged: packer.gameName = text }
                                TextFieldRow { width: parent.width; label: "Version"; text: packer.version; onTextChanged: packer.version = text }
                                TextFieldRow { width: parent.width; label: "Publisher"; text: packer.publisher; onTextChanged: packer.publisher = text }

                                Row {
                                    width: parent.width
                                    spacing: 12
                                    PrimaryButton { width: 170; text: "Source Folder"; accent: window.accent; onClicked: sourceDialog.open() }
                                    Text { text: packer.sourceFolder || "No source selected"; color: "#C8D0DD"; width: parent.width - 182; elide: Text.ElideMiddle; anchors.verticalCenter: parent.verticalCenter }
                                }
                                Row {
                                    width: parent.width
                                    spacing: 12
                                    PrimaryButton { width: 170; text: "Output Folder"; accent: window.accent; onClicked: outputDialog.open() }
                                    Text { text: packer.outputFolder || "No output selected"; color: "#C8D0DD"; width: parent.width - 182; elide: Text.ElideMiddle; anchors.verticalCenter: parent.verticalCenter }
                                }
                            }
                        }
                    }

                    Item {
                        Rectangle {
                            width: parent.width
                            height: 360
                            radius: 8
                            color: "#151922"
                            border.color: "#252D3B"
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 18
                                Text { text: "Profile"; color: "#F4F7FB"; font.pixelSize: 18; font.weight: Font.DemiBold }
                                Row {
                                    spacing: 10
                                    Repeater {
                                        model: ["fast", "balanced", "maximum", "extreme"]
                                        delegate: Button {
                                            text: modelData.charAt(0).toUpperCase() + modelData.slice(1)
                                            width: 130
                                            height: 42
                                            onClicked: packer.profile = modelData
                                            background: Rectangle {
                                                radius: 8
                                                color: packer.profile === modelData ? window.accent : "#10141D"
                                                border.color: "#2A3140"
                                            }
                                            contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 14 }
                                        }
                                    }
                                }
                                MetricCard { width: 260; label: "Selected"; value: packer.profile; accent: window.accent }
                                Text { width: parent.width; text: "Threads: Auto    Dictionary: Profile Default    Split: 4 GB"; color: "#9AA4B2"; font.pixelSize: 14 }
                            }
                        }
                    }

                    Item {
                        Rectangle {
                            width: parent.width
                            height: 360
                            radius: 8
                            color: "#151922"
                            border.color: "#252D3B"
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 14
                                TextFieldRow { width: parent.width; label: "Accent color"; text: packer.accentColor; onTextChanged: packer.accentColor = text }
                                TextFieldRow { width: parent.width; label: "Installer description"; text: packer.description; onTextChanged: packer.description = text }
                                Rectangle {
                                    width: parent.width
                                    height: 150
                                    radius: 8
                                    color: "#0F131B"
                                    border.color: "#2A3140"
                                    Text {
                                        anchors.centerIn: parent
                                        text: packer.gameName
                                        color: "#F4F7FB"
                                        font.pixelSize: 34
                                        font.weight: Font.Bold
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        Column {
                            width: parent.width
                            spacing: 16
                            Rectangle {
                                width: parent.width
                                height: 18
                                radius: 8
                                color: "#10141D"
                                Rectangle {
                                    width: parent.width * Math.max(0, Math.min(1, packer.progress / 100))
                                    height: parent.height
                                    radius: 8
                                    color: window.accent
                                    Behavior on width { NumberAnimation { duration: 120 } }
                                }
                            }
                            Text { text: Math.round(packer.progress) + "%  " + packer.currentFile; color: "#C8D0DD"; font.pixelSize: 14; width: parent.width; elide: Text.ElideMiddle }
                            Rectangle {
                                width: parent.width
                                height: 460
                                radius: 8
                                color: "#090C11"
                                border.color: "#202838"
                                ScrollView {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    TextArea {
                                        text: packer.log
                                        readOnly: true
                                        color: "#C8D0DD"
                                        font.family: "Consolas"
                                        font.pixelSize: 13
                                        background: null
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
