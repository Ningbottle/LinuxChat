import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../styles"
import "../components"

Rectangle {
    color: Theme.colors.canvas

    Rectangle {
        anchors.centerIn: parent
        width: 380; height: 480; radius: Theme.radius.lg !== undefined ? Theme.radius.lg : 12
        color: Theme.colors.surface
        border.width: 1; border.color: Theme.colors.border

        // subtle shadow if possible
        // shadow could be added here depending on Qt5Compat/Qt6 effects

        ColumnLayout {
            anchors.fill: parent; anchors.margins: 36; spacing: Theme.space.md

            Text { 
                text: "LinuxChat"
                font.family: Theme.fonts.title
                font.pixelSize: 24
                font.weight: Font.Bold
                color: Theme.colors.text
                Layout.alignment: Qt.AlignHCenter 
            }
            Text { 
                text: "网络即时通信工具"
                font.family: Theme.fonts.body
                font.pixelSize: Theme.fonts.bodySize
                color: Theme.colors.muted
                Layout.alignment: Qt.AlignHCenter 
            }
            Item { Layout.preferredHeight: Theme.space.md }

            LCButton { 
                Layout.fillWidth: true
                text: loginController.state === 0 ? "连接服务器" : (loginController.state === 1 ? "连接中..." : "已连接")
                enabled: loginController.state === 0
                onClicked: loginController.connectToServer("120.55.63.32", 8080)
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.colors.border }

            LCTextField { id: uf; Layout.fillWidth: true; placeholderText: "用户名" }
            LCTextField { id: pwf; Layout.fillWidth: true; placeholderText: "密码"; echoMode: TextInput.Password; Keys.onReturnPressed: doLogin() }
            Item { Layout.preferredHeight: 4 }

            RowLayout { spacing: 10
                LCButton { Layout.fillWidth: true; text: "登 录"; enabled: loginController.state === 2; onClicked: doLogin() }
                LCButton { Layout.fillWidth: true; text: "注 册"; enabled: loginController.state === 2; onClicked: loginController.registerUser(uf.text.trim(), pwf.text.trim()) }
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

    function doLogin() {
        var u = uf.text.trim(), p = pwf.text.trim();
        if (u.length === 0 || p.length === 0) return;
        loginController.login(u, p);
    }
}
