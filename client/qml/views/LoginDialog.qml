import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#FAFAFA"

    Rectangle {
        anchors.centerIn: parent
        width: 380; height: 480; radius: 12
        color: "#FFFFFF"; border.width: 1; border.color: "#E5E5EA"

        ColumnLayout {
            anchors.fill: parent; anchors.margins: 36; spacing: 12

            Text { text: "LinuxChat"; font.pixelSize: 22; font.bold: true; color: "#1A1A1A"; Layout.alignment: Qt.AlignHCenter }
            Text { text: "网络即时通信工具"; font.pixelSize: 13; color: "#8E8E93"; Layout.alignment: Qt.AlignHCenter }
            Item { Layout.preferredHeight: 8 }

            RowLayout { spacing: 6
                TextField { id: hf; Layout.fillWidth: true; placeholderText: "地址"; text: "120.55.63.32" }
                TextField { id: pf; Layout.preferredWidth: 60; placeholderText: "端口"; text: "8080" }
                Button { text: "连接"; enabled: loginController.state === 0; onClicked: loginController.connectToServer(hf.text.trim(), parseInt(pf.text, 10)) }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: "#E5E5EA" }

            TextField { id: uf; Layout.fillWidth: true; placeholderText: "用户名" }
            TextField { id: pwf; Layout.fillWidth: true; placeholderText: "密码"; echoMode: TextInput.Password; Keys.onReturnPressed: doLogin() }
            Item { Layout.preferredHeight: 4 }

            RowLayout { spacing: 10
                Button { Layout.fillWidth: true; text: "登 录"; enabled: loginController.state === 2; onClicked: doLogin() }
                Button { Layout.fillWidth: true; text: "注 册"; enabled: loginController.state === 2; onClicked: loginController.registerUser(uf.text.trim(), pwf.text.trim()) }
            }

            Text { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: loginController.statusText; color: loginController.isError ? "#EF4444" : "#10B981"; visible: text.length > 0 }
        }
    }

    function doLogin() {
        var u = uf.text.trim(), p = pwf.text.trim();
        if (u.length === 0 || p.length === 0) return;
        loginController.login(u, p);
    }
}
