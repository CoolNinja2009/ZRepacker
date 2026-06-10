import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import RepackInstaller
import "components"

ApplicationWindow {
    id: window
    width: 980
    height: 640
    minimumWidth: 860
    minimumHeight: 560
    visible: true
    title: installer.title
    color: "#0B0D10"

    InstallerController { id: installer }

    property color accent: "#4F8CFF"

    FolderDialog {
        id: installDialog
        title: "Select install folder"
        onAccepted: installer.installFolder = selectedFolder.toString().replace("file:///", "").replace(/\//g, "\\")
    }

    Rectangle {
        anchors.fill: parent
        color: "#0B0D10"

        Rectangle {
            anchors.fill: parent
            anchors.margins: 24
            radius: 8
            color: "#10141D"
            border.color: "#242C3B"

            Row {
                anchors.fill: parent

                Rectangle {
                    width: 330
                    height: parent.height
                    radius: 8
                    color: "#151B27"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 28
                        spacing: 18

                        Rectangle {
                            width: 88
                            height: 88
                            radius: 8
                            color: window.accent
                            Text {
                                anchors.centerIn: parent
                                text: installer.gameName.length > 0 ? installer.gameName.charAt(0) : "G"
                                color: "white"
                                font.pixelSize: 42
                                font.weight: Font.Bold
                            }
                        }

                        Text {
                            text: installer.gameName
                            color: "#F4F7FB"
                            font.pixelSize: 34
                            font.weight: Font.Bold
                            width: parent.width
                            wrapMode: Text.WordWrap
                        }

                        Text {
                            text: "Version " + installer.version
                            color: "#9AA4B2"
                            font.pixelSize: 14
                        }

                        Text {
                            text: installer.description
                            color: "#C8D0DD"
                            font.pixelSize: 14
                            width: parent.width
                            wrapMode: Text.WordWrap
                        }

                        Item { width: 1; height: 10 }

                        StatPill { width: parent.width; label: "Install Size"; value: installer.installSize }
                        StatPill { width: parent.width; label: "Required Space"; value: installer.requiredSpace }

                        Item { width: 1; height: 1 }
                    }
                }

                Item {
                    width: parent.width - 330
                    height: parent.height

                    Column {
                        anchors.fill: parent
                        anchors.margins: 28
                        spacing: 22

                        Text {
                            text: installer.finished ? "Complete" : installer.installing ? "Installing" : "Ready to Install"
                            color: "#F4F7FB"
                            font.pixelSize: 28
                            font.weight: Font.Bold
                        }

                        Rectangle {
                            width: parent.width
                            height: 88
                            radius: 8
                            color: "#151922"
                            border.color: "#252D3B"

                            Row {
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 12
                                Column {
                                    width: parent.width - 146
                                    spacing: 8
                                    Text { text: "Install Directory"; color: "#9AA4B2"; font.pixelSize: 12 }
                                    Text {
                                        text: installer.installFolder
                                        color: "#F4F7FB"
                                        font.pixelSize: 14
                                        width: parent.width
                                        elide: Text.ElideMiddle
                                    }
                                }
                                PrimaryButton {
                                    width: 130
                                    text: "Browse"
                                    accent: window.accent
                                    enabled: !installer.installing
                                    anchors.verticalCenter: parent.verticalCenter
                                    onClicked: installDialog.open()
                                }
                            }
                        }

                        Rectangle {
                            width: parent.width
                            height: 126
                            radius: 8
                            color: "#151922"
                            border.color: "#252D3B"

                            Column {
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 12
                                Text { text: "Performance"; color: "#F4F7FB"; font.pixelSize: 16; font.weight: Font.DemiBold }
                                Row {
                                    spacing: 10
                                    ModeButton { text: "Auto"; selected: installer.performanceMode === "auto"; accent: window.accent; onClicked: installer.performanceMode = "auto" }
                                    ModeButton { text: "Normal"; selected: installer.performanceMode === "normal"; accent: window.accent; onClicked: installer.performanceMode = "normal" }
                                    ModeButton { text: "Low RAM"; selected: installer.performanceMode === "lowram"; accent: window.accent; onClicked: installer.performanceMode = "lowram" }
                                    ModeButton { text: "High Perf"; selected: installer.performanceMode === "highperf"; accent: window.accent; onClicked: installer.performanceMode = "highperf" }
                                }
                                Text { text: "Estimated RAM: " + installer.ramEstimate; color: "#9AA4B2"; font.pixelSize: 12 }
                            }
                        }

                        Column {
                            width: parent.width
                            spacing: 10
                            Rectangle {
                                width: parent.width
                                height: 18
                                radius: 8
                                color: "#090C11"
                                border.color: "#202838"
                                Rectangle {
                                    width: parent.width * Math.max(0, Math.min(1, installer.progress / 100))
                                    height: parent.height
                                    radius: 8
                                    color: window.accent
                                    Behavior on width { NumberAnimation { duration: 120 } }
                                }
                            }
                            Text {
                                text: Math.round(installer.progress) + "%  " + installer.currentFile
                                color: "#C8D0DD"
                                font.pixelSize: 13
                                width: parent.width
                                elide: Text.ElideMiddle
                            }
                            Text {
                                text: installer.status
                                color: installer.finished ? "#4ADE80" : "#9AA4B2"
                                font.pixelSize: 13
                                width: parent.width
                                elide: Text.ElideRight
                            }
                        }

                        Row {
                            spacing: 12
                            PrimaryButton {
                                width: 170
                                text: installer.installing ? "Installing" : "Install"
                                accent: window.accent
                                enabled: !installer.installing
                                onClicked: installer.startInstall()
                            }
                            Button {
                                width: 150
                                height: 44
                                text: "Verify"
                                enabled: !installer.installing
                                onClicked: installer.verifyBins()
                                background: Rectangle { radius: 8; color: "#151922"; border.color: "#2A3140" }
                                contentItem: Text { text: parent.text; color: "#F4F7FB"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 14 }
                            }
                        }
                    }
                }
            }
        }
    }
}
