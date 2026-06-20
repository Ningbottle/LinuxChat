import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    visible: true
    width: 900
    height: 640
    minimumWidth: 600
    minimumHeight: 400
    title: "LinuxChat"
    color: "#F5F5F5"

    property bool wasAuthenticated: false

    StackView {
        id: stackView
        anchors.fill: parent
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
}
