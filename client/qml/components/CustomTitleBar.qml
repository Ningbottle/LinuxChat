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

    // For dragging the window
    DragHandler {
        onActiveChanged: if (active && window) window.startSystemMove()
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
                width: 40; height: 40
                color: minHover.hovered ? Theme.colors.surfaceHover : "transparent"
                
                HoverHandler { id: minHover }
                TapHandler { onTapped: if (window) window.showMinimized() }

                Rectangle {
                    width: 10; height: 1
                    color: Theme.colors.text
                    anchors.centerIn: parent
                }
            }

            // Maximize/Restore
            Rectangle {
                width: 40; height: 40
                color: maxHover.hovered ? Theme.colors.surfaceHover : "transparent"
                
                HoverHandler { id: maxHover }
                TapHandler {
                    onTapped: {
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
                    border.color: Theme.colors.text
                    border.width: 1
                    anchors.centerIn: parent
                }
            }

            // Close
            Rectangle {
                width: 40; height: 40
                color: closeHover.hovered ? Theme.colors.error : "transparent"
                
                HoverHandler { id: closeHover }
                TapHandler { onTapped: Qt.quit() }

                Text {
                    text: "×"
                    font.pixelSize: 20
                    color: closeHover.hovered ? "#FFFFFF" : Theme.colors.text
                    anchors.centerIn: parent
                }
            }
        }
    }
}
