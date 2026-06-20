import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#F5F5F5"

    RowLayout {
        anchors.fill: parent; spacing: 0

        // Sidebar
        Rectangle {
            Layout.preferredWidth: 260; Layout.fillHeight: true; color: "#FAFAFA"

            ColumnLayout {
                anchors.fill: parent; spacing: 0

                // User header
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 56; color: "#F0F0F0"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 10
                        Rectangle {
                            Layout.preferredWidth: 32; Layout.preferredHeight: 32; radius: 16; color: "#7C8CF0"
                            Text { anchors.centerIn: parent; text: chatBackend.currentUser ? chatBackend.currentUser.charAt(0).toUpperCase() : "U"; color: "#FFF"; font.pixelSize: 14; font.bold: true }
                        }
                        Column {
                            spacing: 2
                            Text { text: chatBackend.currentUser || "用户"; font.pixelSize: 14; font.bold: true; color: "#1A1A1A" }
                            Text { text: "在线"; font.pixelSize: 11; color: "#34C759" }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }

                // Online users
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 28; color: "transparent"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                        Text { text: "在线用户"; font.pixelSize: 11; font.bold: true; color: "#8E8E93" }
                        Item { Layout.fillWidth: true }
                    }
                }

                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true; model: chatBackend.onlineUsers
                    delegate: Rectangle {
                        width: parent.width; height: 34; color: "transparent"
                        RowLayout {
                            anchors.fill: parent; anchors.margins: 10; spacing: 8
                            Rectangle {
                                Layout.preferredWidth: 22; Layout.preferredHeight: 22; radius: 11; color: "#6EB5A6"
                                Text { anchors.centerIn: parent; text: (model.username || "?").charAt(0).toUpperCase(); color: "#FFF"; font.pixelSize: 9; font.bold: true }
                            }
                            Text { text: model.username || ""; font.pixelSize: 13; color: "#1A1A1A" }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                // Logout
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 38; color: lm.containsMouse ? "#FEF2F2" : "transparent"
                    MouseArea { id: lm; anchors.fill: parent; hoverEnabled: true; onClicked: { chatBackend.logout(); chatBackend.disconnectFromServer() } }
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12
                        Text { text: "退出登录"; font.pixelSize: 13; color: lm.containsMouse ? "#EF4444" : "#8E8E93" }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }

        Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: "#E5E5EA" }

        // Chat area
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; color: "#F5F5F5"

            ColumnLayout {
                anchors.fill: parent; spacing: 0

                // Header
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 52; color: "#FFFFFF"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 14; spacing: 10
                        Text { text: "公共聊天室"; font.pixelSize: 15; font.bold: true; color: "#1A1A1A" }
                        Item { Layout.fillWidth: true }
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#E5E5EA" }

                // Messages
                ListView {
                    id: mv
                    Layout.fillWidth: true; Layout.fillHeight: true
                    clip: true; model: chatBackend.roomMessages
                    spacing: 4; leftMargin: 16; rightMargin: 16; topMargin: 12; bottomMargin: 12

                    Text { anchors.centerIn: parent; text: "暂无消息"; font.pixelSize: 14; color: "#8E8E93"; visible: mv.count === 0 }

                    onCountChanged: { if (atYEnd || count === 1) Qt.callLater(positionViewAtEnd) }

                    delegate: Rectangle {
                        width: mv.width; height: 68; color: "transparent"

                        // Avatar - always visible
                        Rectangle {
                            x: 4; y: 4; width: 28; height: 28; radius: 14; color: "#7C8CF0"
                            Text { anchors.centerIn: parent; text: (model.sender || "?").charAt(0).toUpperCase(); color: "#FFF"; font.pixelSize: 11; font.bold: true }
                        }

                        // Sender name
                        Text {
                            x: 40; y: 4; width: mv.width - 80
                            text: model.sender || ""
                            font.pixelSize: 11; font.bold: true; color: "#8E8E93"
                            elide: Text.ElideRight
                        }

                        // Content - use mv.width not parent.width
                        Text {
                            x: 40; y: 22; width: mv.width - 80
                            text: model.content || ""
                            font.pixelSize: 14; color: "#1A1A1A"
                            wrapMode: Text.Wrap
                        }

                        // Timestamp
                        Text {
                            x: mv.width - 80; y: 50
                            text: model.timestamp || ""
                            font.pixelSize: 10; color: "#AEAEB2"
                        }
                    }
                }

                // Input
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 64; color: "#FFFFFF"; border.width: 1; border.color: "#E5E5EA"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 10; spacing: 10
                        Rectangle {
                            Layout.fillWidth: true; Layout.fillHeight: true; radius: 4; color: "#F5F5F5"; border.width: 1; border.color: "#E5E5EA"
                            TextArea {
                                id: inp; anchors.fill: parent; anchors.margins: 8
                                placeholderText: "输入消息..."; font.pixelSize: 14; color: "#1A1A1A"
                                background: Rectangle { color: "transparent" }
                            }
                        }
                        Button {
                            id: sb; Layout.preferredWidth: 40; Layout.preferredHeight: 40
                            enabled: inp.text.trim().length > 0; onClicked: send()
                            background: Rectangle { radius: 4; color: sb.enabled ? "#8E8E93" : "#E5E5EA" }
                            contentItem: Text { text: "↑"; font.pixelSize: 18; font.bold: true; color: "#FFF"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        }
                    }
                }
            }
        }
    }

    function send() { var t = inp.text.trim(); if (t.length === 0) return; chatBackend.sendMessage(t); inp.text = ""; }
    Component.onCompleted: chatBackend.requestHistory("__room__")
}
