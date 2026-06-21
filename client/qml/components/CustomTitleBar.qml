import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import "../styles"

Rectangle {
    id: root
    height: 40
    color: "transparent"

    property string title: "LinuxChat"
    property Window window: null

    property point clickPos: "0,0"

    // For dragging the window manually via OS
    MouseArea {
        anchors.fill: parent
        
        onPressed: function(mouse) {
            if (window) {
                window.startSystemMove()
            }
        }
        
        onDoubleClicked: {
            if (window) {
                if (window.visibility === Window.Maximized) {
                    window.showNormal()
                } else {
                    window.showMaximized()
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing.md
        spacing: 0

        // Spacer to push controls to the right
        Item { Layout.fillWidth: true }

        // Window Controls
        RowLayout {
            spacing: 0
            Layout.alignment: Qt.AlignVCenter

            // Minimize
            Rectangle {
                Layout.preferredWidth: 46; Layout.preferredHeight: 40
                color: minArea.containsMouse ? "rgba(0,0,0,0.1)" : "transparent"
                
                MouseArea {
                    id: minArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: if (window) window.showMinimized()
                }

                Rectangle {
                    width: 12; height: 2
                    color: minArea.containsMouse ? "#1e293b" : "#475569" // Dark slate
                    anchors.centerIn: parent
                }
            }

            // Maximize/Restore
            Rectangle {
                Layout.preferredWidth: 46; Layout.preferredHeight: 40
                color: maxArea.containsMouse ? "rgba(0,0,0,0.1)" : "transparent"
                
                MouseArea {
                    id: maxArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (window) {
                            if (window.visibility === Window.Maximized) {
                                window.showNormal()
                            } else {
                                window.showMaximized()
                            }
                        }
                    }
                }

                Rectangle {
                    width: 12; height: 12
                    color: "transparent"
                    border.color: maxArea.containsMouse ? "#1e293b" : "#475569"
                    border.width: 2
                    anchors.centerIn: parent
                }
            }

            // Close
            Rectangle {
                Layout.preferredWidth: 46; Layout.preferredHeight: 40
                color: closeArea.containsMouse ? "#E81123" : "transparent"
                
                MouseArea {
                    id: closeArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: Qt.quit()
                }

                Text {
                    text: "✕"
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    color: closeArea.containsMouse ? "#FFFFFF" : "#475569"
                    anchors.centerIn: parent
                }
            }
        }
    }
}
