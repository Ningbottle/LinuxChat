import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#F5F5F5"

    property string _localStatus: ""
    property bool   _localIsError: false

    Rectangle {
        id: card
        anchors.centerIn: parent
        width: Math.min(400, parent.width - 40)
        height: contentColumn.implicitHeight + 60
        radius: 12
        color: "#FFFFFF"
        border.width: 1
        border.color: "#E5E7EB"

        ColumnLayout {
            id: contentColumn
            anchors {
                left: parent.left; right: parent.right
                verticalCenter: parent.verticalCenter; margins: 30
            }
            spacing: 12

            Label {
                text: "LinuxChat"
                font.pixelSize: 22; font.bold: true
                color: "#1A1A1A"
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                text: "网络即时通信工具"
                font.pixelSize: 13
                color: "#8E8E93"
                Layout.alignment: Qt.AlignHCenter
            }

            Item { Layout.preferredHeight: 8 }

            RowLayout {
                spacing: 8
                TextField {
                    id: hostField
                    Layout.fillWidth: true
                    placeholderText: "服务器地址"
                    text: "120.55.63.32"
                }
                TextField {
                    id: portField
                    Layout.preferredWidth: 70
                    placeholderText: "端口"
                    text: "8080"
                }
                Button {
                    text: "连接"
                    enabled: loginController.state === 0
                    onClicked: loginController.connectToServer(hostField.text.trim(), parseInt(portField.text, 10))
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#E5E7EB"
            }

            TextField {
                id: usernameField
                Layout.fillWidth: true
                placeholderText: "用户名"
            }
            TextField {
                id: passwordField
                Layout.fillWidth: true
                placeholderText: "密码"
                echoMode: TextInput.Password
                Keys.onReturnPressed: doLogin()
                Keys.onEnterPressed: doLogin()
            }

            RowLayout {
                spacing: 12
                Button {
                    Layout.fillWidth: true
                    text: "登录"
                    enabled: loginController.state === 2
                    onClicked: doLogin()
                }
                Button {
                    Layout.fillWidth: true
                    text: "注册"
                    enabled: loginController.state === 2
                    onClicked: loginController.registerUser(usernameField.text.trim(), passwordField.text.trim())
                }
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: _localStatus.length > 0 ? _localStatus : loginController.statusText
                color: (_localIsError || loginController.isError) ? "#EF4444" : "#10B981"
                visible: text.length > 0
            }
        }
    }

    function doLogin() {
        var user = usernameField.text.trim();
        var pass = passwordField.text.trim();
        if (user.length === 0 || pass.length === 0) {
            _localStatus = "请输入用户名和密码";
            _localIsError = true;
            return;
        }
        loginController.login(user, pass);
    }
}
