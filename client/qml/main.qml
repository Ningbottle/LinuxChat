import QtQuick
import QtQuick.Controls
import "styles"
import "components"

ApplicationWindow {
    id: root
    visible: true
    width: 960
    height: 680
    minimumWidth: 720
    minimumHeight: 480
    title: "LinuxChat"
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint

    property bool wasAuthenticated: false

    CustomTitleBar {
        id: titleBar
        window: root
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        z: 999
    }

    // Resize Handles for Frameless Window
    MouseArea {
        width: 8; anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom
        anchors.topMargin: 8; anchors.bottomMargin: 8
        cursorShape: Qt.SizeHorCursor; onPressed: root.startSystemResize(Qt.LeftEdge)
    }
    MouseArea {
        width: 8; anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom
        anchors.topMargin: 8; anchors.bottomMargin: 8
        cursorShape: Qt.SizeHorCursor; onPressed: root.startSystemResize(Qt.RightEdge)
    }
    MouseArea {
        height: 8; anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        anchors.leftMargin: 8; anchors.rightMargin: 8
        cursorShape: Qt.SizeVerCursor; onPressed: root.startSystemResize(Qt.TopEdge)
    }
    MouseArea {
        height: 8; anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
        anchors.leftMargin: 8; anchors.rightMargin: 8
        cursorShape: Qt.SizeVerCursor; onPressed: root.startSystemResize(Qt.BottomEdge)
    }
    MouseArea {
        width: 8; height: 8; anchors.left: parent.left; anchors.top: parent.top
        cursorShape: Qt.SizeFDiagCursor; onPressed: root.startSystemResize(Qt.TopEdge | Qt.LeftEdge)
    }
    MouseArea {
        width: 8; height: 8; anchors.right: parent.right; anchors.top: parent.top
        cursorShape: Qt.SizeBDiagCursor; onPressed: root.startSystemResize(Qt.TopEdge | Qt.RightEdge)
    }
    MouseArea {
        width: 8; height: 8; anchors.left: parent.left; anchors.bottom: parent.bottom
        cursorShape: Qt.SizeBDiagCursor; onPressed: root.startSystemResize(Qt.BottomEdge | Qt.LeftEdge)
    }
    MouseArea {
        width: 8; height: 8; anchors.right: parent.right; anchors.bottom: parent.bottom
        cursorShape: Qt.SizeFDiagCursor; onPressed: root.startSystemResize(Qt.BottomEdge | Qt.RightEdge)
    }

    StackView {
        id: stackView
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        initialItem: "views/LoginDialog.qml"
    }

    Connections {
        target: loginController
        function onStateChanged() {
            if (loginController.state === 3) {
                wasAuthenticated = true;
                stackView.push("views/ChatWindow.qml");
            } else if (loginController.state === 0 && wasAuthenticated) {
                wasAuthenticated = false;
                stackView.pop(null);
            }
        }
    }

    Component.onCompleted: {
        console.log("LinuxChat started");
    }
}
