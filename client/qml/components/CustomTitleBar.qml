import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import "../styles"

Rectangle {
    id: root
    height: 40
    color: "transparent"
    z: 999 // Ensure it's above other content

    // We use a TapHandler and DragHandler to emulate native window dragging
    TapHandler {
        onTapped: {
            if (tapCount === 2) {
                toggleMaximized()
            }
        }
        gesturePolicy: TapHandler.DragThreshold
    }
    DragHandler {
        onActiveChanged: if (active) Window.window.startSystemMove()
    }

    function toggleMaximized() {
        if (Window.window.visibility === Window.Maximized) {
            Window.window.showNormal()
        } else {
            Window.window.showMaximized()
        }
    }

    RowLayout {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        spacing: 0

        // Minimize button
        Rectangle {
            width: 46; height: 40
            color: minMouse.containsMouse ? Theme.colors.border : "transparent"
            Text {
                anchors.centerIn: parent
                text: "—"
                color: Theme.colors.text
                font.pixelSize: 12
            }
            MouseArea {
                id: minMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: Window.window.showMinimized()
            }
        }

        // Maximize button
        Rectangle {
            width: 46; height: 40
            color: maxMouse.containsMouse ? Theme.colors.border : "transparent"
            Text {
                anchors.centerIn: parent
                text: Window.window.visibility === Window.Maximized ? "❐" : "□"
                color: Theme.colors.text
                font.pixelSize: 16
            }
            MouseArea {
                id: maxMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: toggleMaximized()
            }
        }

        // Close button
        Rectangle {
            width: 46; height: 40
            color: closeMouse.containsMouse ? "#E81123" : "transparent"
            Text {
                anchors.centerIn: parent
                text: "✕"
                color: closeMouse.containsMouse ? "#FFFFFF" : Theme.colors.text
                font.pixelSize: 16
            }
            MouseArea {
                id: closeMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: Qt.quit()
            }
        }
    }

    // Title text
    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 16
        text: Window.window.title
        font.family: Theme.fonts.body
        font.pixelSize: 13
        color: Theme.colors.textMuted
    }
}
