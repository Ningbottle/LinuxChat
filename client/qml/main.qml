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
