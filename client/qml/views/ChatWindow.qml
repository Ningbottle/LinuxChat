import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import "../styles"
import "../components"

Rectangle {
    id: root
    color: "#f1f5f9" // Fallback light background

    // --- BACKGROUND IMAGE ---
    Item {
        anchors.fill: parent
        
        Image {
            id: bgImage
            fillMode: Image.PreserveAspectCrop
            
            property int currentBgIndex: 0
            property var images: [
                "qrc:/images/img1_grass.png",
                "qrc:/images/img2_blue_moon_bright.png",
                "qrc:/images/img3_blue_moon_dark.png",
                "qrc:/images/img4_blue_moon_new.png",
                "qrc:/images/img5_mountain.png",
                "qrc:/images/img6_bamboo.png"
            ]
            source: images[currentBgIndex]
            
            Component.onCompleted: {
                currentBgIndex = Math.floor(Math.random() * images.length);
            }
            
            width: parent.width + 100
            height: parent.height + 100
            x: -50; y: -50
            opacity: 0.9
            
            // Subtle breathing/wind motion
            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation { to: 0; duration: 40000; easing.type: Easing.InOutSine }
                NumberAnimation { to: -50; duration: 35000; easing.type: Easing.InOutSine }
            }
            SequentialAnimation on scale {
                loops: Animation.Infinite
                NumberAnimation { to: 1.05; duration: 38000; easing.type: Easing.InOutSine }
                NumberAnimation { to: 1.0; duration: 42000; easing.type: Easing.InOutSine }
            }
        }
        
        // Light overlay to ensure text readability
        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(255/255, 255/255, 255/255, 0.3)
        }
    }

    SplitView {
        id: splitView
        anchors.fill: parent
        anchors.margins: 20 // Margin to show background around the main window
        orientation: Qt.Horizontal

        // Main glass container background
        Rectangle {
            z: -1
            anchors.fill: parent
            radius: 16
            color: Qt.rgba(255/255, 255/255, 255/255, 0.45) // bg-glass-panel
            border.width: 1
            border.color: Qt.rgba(255/255, 255/255, 255/255, 0.4)
            
            // Inner shadow / glow
            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: "#20000000"
                shadowBlur: 1.5
                shadowVerticalOffset: 4
            }
        }

        // Custom resizable handle
        handle: Rectangle {
            implicitWidth: 1
            color: Qt.rgba(255/255, 255/255, 255/255, 0.3)
        }

        // --- SIDEBAR ---
        Rectangle {
            id: sidebarItem
            SplitView.preferredWidth: 280
            SplitView.minimumWidth: 200
            SplitView.maximumWidth: 400
            color: Qt.rgba(255/255, 255/255, 255/255, 0.2) // bg-white/20
            radius: 16
            
            // Mask the right corners to be square so it meets the chat area
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 16
                color: Qt.rgba(255/255, 255/255, 255/255, 0.2)
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Current User Info
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    color: Qt.rgba(255/255, 255/255, 255/255, 0.3) // bg-white/30
                    border.width: 1
                    border.color: Qt.rgba(255/255, 255/255, 255/255, 0.3)
                    
                    // Top-left rounded corners
                    radius: 16
                    Rectangle { anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right; height: 16; color: parent.color }
                    Rectangle { anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom; width: 16; color: parent.color }

                    RowLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 12
                        LCAvatar {
                            username: chatBackend.currentUser || "U"
                            size: 40
                            isActive: false
                        }
                        ColumnLayout {
                            spacing: 2
                            Layout.fillWidth: true
                            Text { 
                                text: chatBackend.currentUser || "My Username"; 
                                font.family: Theme.fonts.body; font.pixelSize: 15; font.weight: Font.Medium; color: "#1e293b" 
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            RowLayout {
                                spacing: 6
                                Rectangle { width: 8; height: 8; radius: 4; color: Theme.colors.success }
                                Text { 
                                    text: "Online"; 
                                    font.family: Theme.fonts.body; font.pixelSize: 12; color: Theme.colors.success 
                                }
                            }
                        }
                        
                        // Logout Button (Icon)
                        Rectangle {
                            width: 32; height: 32; radius: 16; color: logoutHover.containsMouse ? Qt.rgba(255/255,255/255,255/255,0.5) : "transparent"
                            Text { anchors.centerIn: parent; text: "⎋"; font.pixelSize: 16; color: "#64748B" }
                            MouseArea {
                                id: logoutHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: { chatBackend.logout(); chatBackend.disconnectFromServer() }
                            }
                        }
                    }
                }

                // Search Box
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 64
                    color: "transparent"
                    
                    Rectangle {
                        anchors.fill: parent; anchors.margins: 16
                        radius: 8
                        color: Qt.rgba(255/255, 255/255, 255/255, 0.5) // bg-white/50
                        border.width: 1; border.color: Qt.rgba(255/255, 255/255, 255/255, 0.4)
                        
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 8
                            Text { text: "🔍"; font.pixelSize: 14; color: "#64748B" }
                            TextInput {
                                Layout.fillWidth: true
                                font.family: Theme.fonts.body
                                font.pixelSize: 14
                                color: "#1e293b"
                                verticalAlignment: TextInput.AlignVCenter
                                Text {
                                    anchors.fill: parent
                                    text: "Search contacts..."
                                    color: "#64748B"
                                    font.family: Theme.fonts.body
                                    font.pixelSize: 14
                                    verticalAlignment: Text.AlignVCenter
                                    visible: !parent.text && !parent.activeFocus
                                }
                            }
                        }
                    }
                }

                // Recent Chats Header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: "transparent"
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        anchors.verticalCenter: parent.verticalCenter
                        text: "RECENT CHATS"
                        font.family: Theme.fonts.body
                        font.pixelSize: 11
                        font.weight: Font.Bold
                        font.letterSpacing: 1.5
                        color: "#64748B"
                    }
                }

                // Online users list
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: chatBackend.onlineUsers
                    spacing: 4
                    leftMargin: 12; rightMargin: 12
                    
                    delegate: Item {
                        width: ListView.view.width - 24
                        height: 64
                        
                        Rectangle {
                            id: delegateBg
                            anchors.fill: parent
                            radius: 12
                            // Active style: bg-white/60 + border-white/50. Hover: bg-white/40
                            color: hoverArea.containsMouse ? Qt.rgba(255/255,255/255,255/255,0.4) : "transparent"
                            border.width: 1
                            border.color: hoverArea.containsMouse ? Qt.rgba(255/255,255/255,255/255,0.2) : "transparent"
                            
                            Behavior on color { ColorAnimation { duration: 150 } }
                            
                            RowLayout {
                                anchors.fill: parent; anchors.margins: 10; spacing: 12
                                
                                // Avatar
                                Item {
                                    width: 44; height: 44
                                    LCAvatar {
                                        anchors.centerIn: parent
                                        username: model.username || "?"
                                        size: 44
                                        isActive: false
                                    }
                                }
                                
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 2
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Text { 
                                            text: model.username || ""; 
                                            font.family: Theme.fonts.body; font.pixelSize: 14; font.weight: Font.Medium; color: "#1e293b" 
                                            Layout.fillWidth: true
                                        }
                                        Text {
                                            text: "Now"
                                            font.family: Theme.fonts.body; font.pixelSize: 10; font.weight: Font.Medium; color: Theme.colors.accent
                                        }
                                    }
                                    Text { 
                                        text: "Active in chat"; 
                                        font.family: Theme.fonts.body; font.pixelSize: 12; color: "#64748B" 
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
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
            }
        }

        // --- MAIN CHAT AREA ---
        Rectangle {
            SplitView.fillWidth: true
            color: Qt.rgba(255/255, 255/255, 255/255, 0.1) // bg-white/10
            radius: 16
            
            // Mask the left corners to be square
            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 16
                color: Qt.rgba(255/255, 255/255, 255/255, 0.1)
            }

            ColumnLayout {
                anchors.fill: parent; spacing: 0

                // Chat Header
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 64
                    color: Qt.rgba(255/255, 255/255, 255/255, 0.4) // bg-white/40
                    border.width: 1; border.color: Qt.rgba(255/255, 255/255, 255/255, 0.3)
                    
                    // Top-right rounded corners
                    radius: 16
                    Rectangle { anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right; height: 16; color: parent.color }
                    Rectangle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; width: 16; color: parent.color }

                    RowLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 12
                        
                        LCAvatar { username: "Global Chat"; size: 36; isActive: false }
                        
                        ColumnLayout {
                            spacing: 2
                            Text { 
                                text: "Global Chat"; 
                                font.family: Theme.fonts.body; font.pixelSize: 16; font.weight: Font.DemiBold; color: "#1e293b" 
                            }
                            Text { 
                                text: "Active Now"; 
                                font.family: Theme.fonts.body; font.pixelSize: 12; font.weight: Font.Medium; color: Theme.colors.success 
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                }

                // Messages Area
                ListView {
                    id: mv
                    Layout.fillWidth: true; Layout.fillHeight: true
                    clip: true; model: chatBackend.roomMessages
                    spacing: 24
                    leftMargin: 24; rightMargin: 24; 
                    topMargin: 24; bottomMargin: 24

                    Text { 
                        anchors.centerIn: parent; text: "No messages yet. Say hello!"; 
                        font.family: Theme.fonts.body; font.pixelSize: 14; color: "#64748B"; visible: mv.count === 0 
                    }

                    onCountChanged: { if (atYEnd || count === 1) Qt.callLater(positionViewAtEnd) }

                    delegate: Item {
                        width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                        height: msgLayout.height
                        
                        property bool isSelf: model.sender === chatBackend.currentUser
                        
                        RowLayout {
                            id: msgLayout
                            anchors.left: isSelf ? undefined : parent.left
                            anchors.right: isSelf ? parent.right : undefined
                            width: Math.min(parent.width * 0.75, implicitWidth)
                            layoutDirection: isSelf ? Qt.RightToLeft : Qt.LeftToRight
                            spacing: 12
                            
                            // Avatar
                            Item {
                                Layout.alignment: Qt.AlignBottom
                                width: 28; height: 28
                                LCAvatar {
                                    anchors.fill: parent
                                    username: model.sender || ""
                                    size: 28
                                }
                            }
                            
                            // Bubble
                            ColumnLayout {
                                spacing: 4
                                Rectangle {
                                    Layout.maximumWidth: mv.width * 0.65
                                    implicitWidth: msgText.implicitWidth + 24
                                    implicitHeight: msgText.implicitHeight + 20
                                    
                                    color: isSelf ? Qt.rgba(59/255, 130/255, 246/255, 0.9) : Qt.rgba(255/255, 255/255, 255/255, 0.7)
                                    border.color: isSelf ? Qt.rgba(96/255, 165/255, 250/255, 0.5) : Qt.rgba(255/255, 255/255, 255/255, 0.5)
                                    border.width: 1
                                    
                                    radius: 16
                                    // Flatten the bottom corner next to avatar
                                    Rectangle {
                                        visible: !isSelf
                                        anchors.bottom: parent.bottom
                                        anchors.left: parent.left
                                        width: 16; height: 16
                                        color: parent.color
                                        radius: 4
                                    }
                                    Rectangle {
                                        visible: isSelf
                                        anchors.bottom: parent.bottom
                                        anchors.right: parent.right
                                        width: 16; height: 16
                                        color: parent.color
                                        radius: 4
                                    }
                                    
                                    layer.enabled: true
                                    layer.effect: MultiEffect {
                                        shadowEnabled: true
                                        shadowColor: "#15000000"
                                        shadowBlur: 0.2
                                        shadowVerticalOffset: 1
                                    }

                                    Text {
                                        id: msgText
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12
                                        text: model.content || ""
                                        font.family: Theme.fonts.body
                                        font.pixelSize: 14
                                        color: isSelf ? "#ffffff" : "#1e293b"
                                        wrapMode: Text.Wrap
                                    }
                                }
                                
                                Text {
                                    Layout.alignment: isSelf ? Qt.AlignRight : Qt.AlignLeft
                                    Layout.leftMargin: 4; Layout.rightMargin: 4
                                    text: {
                                        if (model.timestamp) {
                                            var d = new Date(model.timestamp);
                                            return d.getHours().toString().padStart(2, '0') + ":" + d.getMinutes().toString().padStart(2, '0');
                                        }
                                        return "";
                                    }
                                    font.family: Theme.fonts.body
                                    font.pixelSize: 10
                                    font.weight: Font.Medium
                                    color: "#64748B"
                                }
                            }
                        }
                    }
                }

                // Input Area
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 76; 
                    color: Qt.rgba(255/255, 255/255, 255/255, 0.4) // bg-white/40
                    border.width: 1; border.color: Qt.rgba(255/255, 255/255, 255/255, 0.3)
                    
                    // Bottom-right rounded corners
                    radius: 16
                    Rectangle { anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right; height: 16; color: parent.color }
                    Rectangle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; width: 16; color: parent.color }

                    RowLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 12
                        
                        Rectangle {
                            Layout.fillWidth: true; Layout.fillHeight: true; 
                            radius: 16
                            color: Qt.rgba(255/255, 255/255, 255/255, 0.6) // bg-white/60
                            border.width: 1; border.color: inp.activeFocus ? Qt.rgba(96/255, 165/255, 250/255, 0.8) : Qt.rgba(255/255, 255/255, 255/255, 0.5)
                            
                            Behavior on border.color { ColorAnimation { duration: 150 } }
                            
                            ScrollView {
                                anchors.fill: parent; anchors.margins: 4
                                TextArea {
                                    id: inp
                                    placeholderText: "Type a message..."
                                    font.family: Theme.fonts.body
                                    font.pixelSize: 14
                                    color: "#1e293b"
                                    background: null
                                    wrapMode: Text.Wrap
                                    topPadding: 10
                                    leftPadding: 12
                                    
                                    Keys.onReturnPressed: (event) => {
                                        if (event.modifiers & Qt.ShiftModifier) {
                                            event.accepted = false;
                                        } else {
                                            send();
                                            event.accepted = true;
                                        }
                                    }
                                }
                            }
                        }
                        
                        Rectangle {
                            Layout.preferredWidth: 44; Layout.preferredHeight: 44
                            radius: 12
                            color: Theme.colors.accent
                            
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                shadowEnabled: true
                                shadowColor: Qt.rgba(59/255, 130/255, 246/255, 0.3)
                                shadowBlur: 0.5
                                shadowVerticalOffset: 2
                            }
                            
                            Text {
                                anchors.centerIn: parent
                                text: "➤"
                                color: "white"
                                font.pixelSize: 18
                                font.bold: true
                            }
                            
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: send()
                                onPressed: parent.opacity = 0.8
                                onReleased: parent.opacity = 1.0
                            }
                        }
                    }
                }
            }
        }
    }

    function send() { 
        var t = inp.text.trim(); 
        if (t.length === 0) return; 
        chatBackend.sendMessage(t); 
        inp.text = ""; 
    }
    
    Component.onCompleted: chatBackend.requestHistory("__room__")
}
