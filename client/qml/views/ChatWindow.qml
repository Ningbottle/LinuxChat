import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import "../styles"
import "../components"

Rectangle {
    color: Theme.colors.canvas

    SplitView {
        id: splitView
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Custom resizable handle styling (wider for easier grabbing)
        handle: Rectangle {
            implicitWidth: 6
            color: "transparent"

            Rectangle {
                anchors.centerIn: parent
                width: 2
                height: parent.height
                color: SplitHandle.pressed ? Theme.colors.accent : (SplitHandle.hovered ? Theme.colors.accentHover : "transparent")
                
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            // Little pill indicator like HTML
            Rectangle {
                anchors.centerIn: parent
                width: 4
                height: 40
                radius: 2
                color: Theme.colors.border
                opacity: SplitHandle.pressed || SplitHandle.hovered ? 0.8 : 0.0
                Behavior on opacity { NumberAnimation { duration: 150 } }
            }
        }

        // Sidebar
        Rectangle {
            id: sidebarItem
            SplitView.preferredWidth: 280
            SplitView.minimumWidth: 200
            SplitView.maximumWidth: 500
            color: Theme.colors.sidebarBg

            // Subtle inner border for glass effect
            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: Theme.colors.border
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // App Header & Search
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "transparent"
                    
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: Theme.space.md; spacing: Theme.space.md
                        
                        RowLayout {
                            Text {
                                text: "LinuxChat"
                                font.family: Theme.fonts.title
                                font.pixelSize: Theme.fonts.titleSize * 1.2
                                font.weight: Font.Bold
                                color: Theme.colors.text
                            }
                            Item { Layout.fillWidth: true }
                        }
                        
                        // Search Box
                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 36
                            radius: 10
                            color: Qt.rgba(0,0,0,0.04)
                            border.width: 1.5; border.color: Qt.rgba(0,0,0,0.06)
                            
                            RowLayout {
                                anchors.fill: parent; anchors.leftMargin: Theme.space.sm; anchors.rightMargin: Theme.space.sm; spacing: Theme.space.xs
                                Text { text: "🔍"; font.pixelSize: 14; color: Theme.colors.muted }
                                TextInput {
                                    Layout.fillWidth: true
                                    font.family: Theme.fonts.body
                                    font.pixelSize: 14
                                    color: Theme.colors.text
                                    verticalAlignment: TextInput.AlignVCenter
                                    Text {
                                        anchors.fill: parent
                                        text: "搜索联系人..."
                                        color: Theme.colors.muted
                                        font.family: Theme.fonts.body
                                        font.pixelSize: 14
                                        verticalAlignment: Text.AlignVCenter
                                        visible: !parent.text && !parent.activeFocus
                                    }
                                }
                            }
                        }
                    }
                }

                // Channels Section (UI detail matching HTML mockup)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "transparent"
                    
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 4
                        Text {
                            text: "CHANNELS"
                            font.family: Theme.fonts.mono
                            font.pixelSize: 11
                            font.weight: Font.Bold
                            font.letterSpacing: 1.5
                            color: Theme.colors.muted
                            Layout.bottomMargin: 4
                        }
                        
                        // Default General Room
                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 36
                            radius: 8
                            color: Theme.colors.sidebarActive
                            border.width: 1; border.color: Theme.colors.border
                            RowLayout {
                                anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 8
                                Text { text: "#"; font.family: Theme.fonts.mono; color: Theme.colors.muted; font.pixelSize: 14 }
                                Text { text: "general"; font.family: Theme.fonts.body; color: Theme.colors.text; font.pixelSize: 14; Layout.fillWidth: true }
                                Rectangle { width: 6; height: 6; radius: 3; color: Theme.colors.accent }
                            }
                        }
                        
                        // Engineering (Decorative)
                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 36
                            radius: 8
                            color: "transparent"
                            RowLayout {
                                anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 8
                                Text { text: "#"; font.family: Theme.fonts.mono; color: Theme.colors.subtle; font.pixelSize: 14 }
                                Text { text: "engineering"; font.family: Theme.fonts.body; color: Theme.colors.muted; font.pixelSize: 14; Layout.fillWidth: true }
                            }
                            MouseArea { anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onEntered: parent.color = Theme.colors.sidebarHover; onExited: parent.color = "transparent" }
                        }
                    }
                }

                // Online Users Header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: "transparent"
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: "DIRECT MESSAGES"
                        font.family: Theme.fonts.mono
                        font.pixelSize: 11
                        font.weight: Font.Bold
                        font.letterSpacing: 1.5
                        color: Theme.colors.muted
                    }
                }

                // Online users list
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: chatBackend.onlineUsers
                    spacing: 4
                    leftMargin: 8; rightMargin: 8
                    
                    delegate: Item {
                        width: ListView.view.width - 16
                        height: 60
                        
                        Rectangle {
                            id: delegateBg
                            anchors.fill: parent
                            radius: 8
                            color: hoverArea.containsMouse ? Theme.colors.sidebarHover : "transparent"
                            
                            Behavior on color { ColorAnimation { duration: 150 } }
                            
                            RowLayout {
                                anchors.fill: parent; anchors.margins: Theme.space.sm; spacing: Theme.space.md
                                
                                // Avatar with online dot
                                Item {
                                    width: 44; height: 44
                                    LCAvatar {
                                        anchors.centerIn: parent
                                        username: model.username || "?"
                                        size: 44
                                        isActive: false // We draw our own pulsing dot below
                                    }
                                    
                                    // Pulsing dot
                                    Rectangle {
                                        width: 12; height: 12; radius: 6
                                        color: Theme.colors.success
                                        border.width: 2; border.color: Theme.colors.surface
                                        anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.margins: -2
                                        
                                        SequentialAnimation on opacity {
                                            loops: Animation.Infinite
                                            NumberAnimation { from: 1.0; to: 0.5; duration: 1000 }
                                            NumberAnimation { from: 0.5; to: 1.0; duration: 1000 }
                                        }
                                    }
                                }
                                
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 2
                                    Text { 
                                        text: model.username || ""; 
                                        font.family: Theme.fonts.title; font.pixelSize: 15; font.weight: Font.Bold; color: Theme.colors.text 
                                    }
                                    Text { 
                                        text: "在线中..."; 
                                        font.family: Theme.fonts.caption; font.pixelSize: 13; color: Theme.colors.muted 
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                            
                            MouseArea {
                                id: hoverArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.colors.border }

                // User card at bottom
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 70; 
                    color: "transparent"
                    
                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 8
                        radius: 10
                        color: Theme.colors.sidebarActive
                    RowLayout {
                        anchors.fill: parent; anchors.margins: Theme.space.md; spacing: Theme.space.md
                        LCAvatar {
                            username: chatBackend.currentUser || "U"
                            size: 40
                            isActive: true
                        }
                        ColumnLayout {
                            spacing: 2
                            Text { 
                                text: chatBackend.currentUser || "用户"; 
                                font.family: Theme.fonts.title; font.pixelSize: 14; font.weight: Font.Bold; color: Theme.colors.text 
                            }
                            Text { 
                                text: "退出登录"; 
                                font.family: Theme.fonts.caption; font.pixelSize: 12; color: Theme.colors.danger 
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { chatBackend.logout(); chatBackend.disconnectFromServer() }
                                }
                            }
                        }
                        Item { Layout.fillWidth: true }
                    }
                    }
                }
            }
        }

        // Chat area
        Rectangle {
            SplitView.fillWidth: true
            color: Theme.colors.canvas

            ColumnLayout {
                anchors.fill: parent; spacing: 0

                // Header
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 64; color: Theme.colors.surface
                    RowLayout {
                        anchors.fill: parent; anchors.margins: Theme.space.md; spacing: Theme.space.md
                        
                        LCAvatar { username: "Chat"; size: 40; isActive: false }
                        
                        ColumnLayout {
                            spacing: 2
                            Text { 
                                text: "公共聊天室"; 
                                font.family: Theme.fonts.title; font.pixelSize: 16; font.weight: Font.Bold; color: Theme.colors.text 
                            }
                            Text { 
                                text: "与世界分享你的声音"; 
                                font.family: Theme.fonts.caption; font.pixelSize: 12; color: Theme.colors.muted 
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                        
                        // Theme Switcher for debugging
                        ComboBox {
                            model: Theme.skinNames
                            currentIndex: Theme.skinNames.indexOf(Theme.currentSkin)
                            onActivated: index => {
                                themeMgr.setSkin(model[index]);
                            }
                        }
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.colors.border }

                // Messages
                ListView {
                    id: mv
                    Layout.fillWidth: true; Layout.fillHeight: true
                    clip: true; model: chatBackend.roomMessages
                    spacing: Theme.space.md; 
                    leftMargin: Theme.space.lg; rightMargin: Theme.space.lg; 
                    topMargin: Theme.space.md; bottomMargin: Theme.space.md

                    Text { 
                        anchors.centerIn: parent; text: "暂无消息"; 
                        font.family: Theme.fonts.body; font.pixelSize: Theme.fonts.bodySize; color: Theme.colors.muted; visible: mv.count === 0 
                    }

                    onCountChanged: { if (atYEnd || count === 1) Qt.callLater(positionViewAtEnd) }

                    delegate: MessageBubble {
                        text: model.content || ""
                        isSelf: model.sender === chatBackend.currentUser
                        sender: model.sender || ""
                        time: model.timestamp || ""
                        hasTail: true
                        width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                    }
                }

                // Input
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 80; 
                    color: Theme.colors.surface; 
                    border.width: 1; border.color: Theme.colors.border
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: Theme.space.md; spacing: Theme.space.md
                        
                        Rectangle {
                            Layout.fillWidth: true; Layout.fillHeight: true; 
                            radius: Theme.bubble.radius; 
                            color: Theme.colors.canvas; 
                            border.width: 1; border.color: Theme.colors.border
                            
                            ScrollView {
                                anchors.fill: parent; anchors.margins: Theme.space.sm
                                TextArea {
                                    id: inp
                                    placeholderText: "输入消息..."
                                    font.family: Theme.fonts.body
                                    font.pixelSize: Theme.fonts.bodySize
                                    color: Theme.colors.text
                                    background: null
                                    wrapMode: Text.Wrap
                                    
                                    Keys.onReturnPressed: (event) => {
                                        if (event.modifiers & Qt.ShiftModifier) {
                                            // Let the default handler add a new line
                                            event.accepted = false;
                                        } else {
                                            send();
                                            event.accepted = true;
                                        }
                                    }
                                }
                            }
                        }
                        
                        LCButton {
                            id: sb; Layout.preferredWidth: 64; Layout.preferredHeight: 48
                            text: "发送"
                            enabled: inp.text.trim().length > 0; onClicked: send()
                        }
                    }
                }
            }
        }
    }

    function send() { var t = inp.text.trim(); if (t.length === 0) return; chatBackend.sendMessage(t); inp.text = ""; }
    Component.onCompleted: chatBackend.requestHistory("__room__")
}
