import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../styles"
import "../components"

Rectangle {
    color: Theme.colors ? Theme.colors.canvas : "#F4F4F9"

    Rectangle {
        anchors.centerIn: parent
        width: 760; height: 480; radius: Theme.radius.lg !== undefined ? Theme.radius.lg : 16
        color: Theme.colors.surface
        border.width: 1; border.color: Theme.colors.border
        clip: true

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // Left side: Branding & Graphic
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 340
                color: Theme.colors.accent
                radius: Theme.radius.lg !== undefined ? Theme.radius.lg : 16
                // Square off the right corners so it connects seamlessly to the right panel
                Rectangle {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 20
                    color: Theme.colors.accent
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 40
                    spacing: 20

                    Image {
                        source: "qrc:/images/globe.svg"
                        sourceSize: Qt.size(64, 64)
                        Layout.alignment: Qt.AlignLeft
                        opacity: 0.9
                    }

                    Item { Layout.fillHeight: true }

                    Text {
                        text: "Welcome to\nLinuxChat."
                        font.family: Theme.fonts.title
                        font.pixelSize: 32
                        font.weight: Font.Bold
                        color: "#FFFFFF"
                        lineHeight: 1.2
                    }

                    Text {
                        text: "Connect securely with friends across platforms in real time."
                        font.family: Theme.fonts.body
                        font.pixelSize: Theme.fonts.bodySize
                        color: "#CCFFFFFF"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                    
                    Item { Layout.preferredHeight: 20 }
                }
            }

            // Right side: Login Form
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.centerIn: parent
                    width: 300
                    spacing: Theme.space.md

                    Text { 
                        text: "Sign In"
                        font.family: Theme.fonts.title
                        font.pixelSize: 24
                        font.weight: Font.Bold
                        color: Theme.colors.text
                        Layout.alignment: Qt.AlignLeft 
                    }
                    
                    Item { Layout.preferredHeight: 10 }

                    LCTextField { 
                        id: uf
                        Layout.fillWidth: true
                        placeholderText: "用户名 (Username)" 
                    }
                    
                    LCTextField { 
                        id: pwf
                        Layout.fillWidth: true
                        placeholderText: "密码 (Password)"
                        echoMode: TextInput.Password
                        Keys.onReturnPressed: doLogin() 
                    }
                    
                    Item { Layout.preferredHeight: 4 }

                    LCButton { 
                        Layout.fillWidth: true
                        text: loginController.state === 0 ? "连接服务器 (Connect)" : (loginController.state === 1 ? "连接中..." : "已连接")
                        enabled: loginController.state === 0
                        onClicked: loginController.connectToServer("120.55.63.32", 8080)
                    }

                    RowLayout { 
                        spacing: 10
                        Layout.fillWidth: true
                        LCButton { 
                            Layout.fillWidth: true
                            text: "登 录"
                            enabled: loginController.state === 2
                            onClicked: doLogin() 
                        }
                        LCButton { 
                            Layout.fillWidth: true
                            text: "注 册"
                            enabled: loginController.state === 2
                            onClicked: loginController.registerUser(uf.text.trim(), pwf.text.trim()) 
                        }
                    }

                    Text { 
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: loginController.statusText
                        font.family: Theme.fonts.caption
                        font.pixelSize: Theme.fonts.captionSize
                        color: loginController.isError ? Theme.colors.danger : Theme.colors.success
                        visible: text.length > 0 
                    }
                }
            }
        }
    }

    function doLogin() {
        var u = uf.text.trim(), p = pwf.text.trim();
        if (u.length === 0 || p.length === 0) return;
        loginController.login(u, p);
    }
}
