import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#F5F5F5"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 52
            color: "#FFFFFF"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                Label {
                    text: "公共聊天室"
                    font.pixelSize: 16; font.bold: true
                    color: "#1A1A1A"
                    Layout.fillWidth: true
                }
                Button {
                    text: "登出"
                    onClicked: {
                        chatBackend.logout();
                        chatBackend.disconnectFromServer();
                    }
                }
            }
        }

        // Message list
        ListView {
            id: messageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: chatBackend.roomMessages
            spacing: 8

            onCountChanged: {
                if (atYEnd || count === 1) Qt.callLater(positionViewAtEnd)
            }

            delegate: Rectangle {
                width: messageList.width
                height: msgCol.implicitHeight + 20
                color: model.isSelf ? "#3B82F6" : "#FFFFFF"
                radius: 8

                ColumnLayout {
                    id: msgCol
                    anchors.fill: parent
                    anchors.margins: 10
                    Label {
                        text: model.sender
                        font.pixelSize: 11; font.bold: true
                        color: model.isSelf ? "#FFFFFF80" : "#8E8E93"
                    }
                    Label {
                        text: model.content
                        font.pixelSize: 14
                        color: model.isSelf ? "#FFFFFF" : "#1A1A1A"
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                    Label {
                        text: model.timestamp
                        font.pixelSize: 10
                        color: model.isSelf ? "#FFFFFF60" : "#8E8E93"
                        Layout.alignment: Qt.AlignRight
                    }
                }
            }
        }

        // Input area
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#FFFFFF"
            border.width: 1
            border.color: "#E5E7EB"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                TextArea {
                    id: inputField
                    Layout.fillWidth: true
                    placeholderText: "输入消息... (Ctrl+Enter 发送)"
                    wrapMode: TextArea.Wrap
                    Keys.onReturnPressed: (e) => {
                        if (e.modifiers & Qt.ControlModifier) sendMessage();
                        else e.accepted = false;
                    }
                }
                Button {
                    text: "发送"
                    enabled: inputField.text.trim().length > 0
                    onClicked: sendMessage()
                }
            }
        }
    }

    function sendMessage() {
        var t = inputField.text.trim();
        if (t.length === 0) return;
        chatBackend.sendMessage(t);
        inputField.text = "";
    }

    Component.onCompleted: chatBackend.requestHistory("__room__")
}
