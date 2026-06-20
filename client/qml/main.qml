import QtQuick
import QtQuick.Controls
import "styles"

ApplicationWindow {
    id: root
    visible: true
    width: 960
    height: 680
    minimumWidth: 720
    minimumHeight: 480
    title: "LinuxChat"
    color: themeMgr.skin().canvas

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

    Connections {
        target: themeMgr
        function onSkinChanged() {
            console.log("Skin changed to:", themeMgr.currentSkin());
            Theme.setSkin(themeMgr.currentSkin());
        }
    }

    Component.onCompleted: {
        console.log("LinuxChat started");
        Theme.setSkin(themeMgr.currentSkin());
    }
}
