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

        Text {
            text: root.title
            font.family: Theme.fonts.body
            font.pixelSize: Theme.fonts.captionSize
            color: Theme.colors.textMuted
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
        }

        // Window Controls
        RowLayout {
            spacing: 0
            Layout.alignment: Qt.AlignVCenter

            // Minimize
            Rectangle {
                Layout.preferredWidth: 40; Layout.preferredHeight: 40
                color: minArea.containsMouse ? "#20FFFFFF" : "transparent"
                
                MouseArea {
                    id: minArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: if (window) window.showMinimized()
                }

                Rectangle {
                    width: 10; height: 1
                    color: minArea.containsMouse ? "#FFFFFF" : "#80FFFFFF"
                    anchors.centerIn: parent
                }
            }

            // Maximize/Restore
            Rectangle {
                Layout.preferredWidth: 40; Layout.preferredHeight: 40
                color: maxArea.containsMouse ? "#20FFFFFF" : "transparent"
                
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
                    width: 10; height: 10
                    color: "transparent"
                    border.color: maxArea.containsMouse ? "#FFFFFF" : "#80FFFFFF"
                    border.width: 1
                    anchors.centerIn: parent
                }
            }

            // Close
            Rectangle {
                Layout.preferredWidth: 40; Layout.preferredHeight: 40
                color: closeArea.containsMouse ? "#E81123" : "transparent"
                
                MouseArea {
                    id: closeArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: Qt.quit()
                }

                Text {
                    text: "×"
                    font.pixelSize: 20
                    color: closeArea.containsMouse ? "#FFFFFF" : "#80FFFFFF"
                    anchors.centerIn: parent
                }
            }
        }
    }
}
