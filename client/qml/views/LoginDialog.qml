import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import "../styles"
import "../components"

Rectangle {
    id: root
    color: Theme.colors.canvas
    clip: true

    // --- ANIMATED GRASSLAND / FLUID HILLS BACKGROUND ---
    
    // Background Orb 1 (Mint)
    Rectangle {
        width: 800; height: 800; radius: 400
        color: Theme.colors.orb2
        opacity: 0.5
        x: -200; y: -200
        SequentialAnimation on x {
            loops: Animation.Infinite
            NumberAnimation { to: 100; duration: 25000; easing.type: Easing.InOutSine }
            NumberAnimation { to: -200; duration: 22000; easing.type: Easing.InOutSine }
        }
    }
    
    // Background Orb 2 (Light Emerald)
    Rectangle {
        width: 1000; height: 1000; radius: 500
        color: Theme.colors.orb1
        opacity: 0.3
        x: parent.width - 600; y: parent.height - 400
        SequentialAnimation on y {
            loops: Animation.Infinite
            NumberAnimation { to: parent.height - 700; duration: 28000; easing.type: Easing.InOutSine }
            NumberAnimation { to: parent.height - 400; duration: 24000; easing.type: Easing.InOutSine }
        }
    }

    // "Moving Grassland" - Parallax overlapping sine waves using QtQuick.Shapes
    // Back Hill (Darker Emerald)
    Shape {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 300
        opacity: 0.6

        ShapePath {
            strokeWidth: 0
            fillColor: Theme.colors.orb3
            startX: 0; startY: 300
            PathLine { x: 0; y: 150 }
            PathCubic { 
                id: wave1
                x: root.width; y: 180
                control1X: root.width * 0.3; control1Y: 50
                control2X: root.width * 0.7; control2Y: 300
            }
            PathLine { x: root.width; y: 300 }
            PathLine { x: 0; y: 300 }
        }

        SequentialAnimation {
            running: true
            loops: Animation.Infinite
            ParallelAnimation {
                NumberAnimation { target: wave1; property: "control1Y"; to: 250; duration: 8000; easing.type: Easing.InOutSine }
                NumberAnimation { target: wave1; property: "control2Y"; to: 100; duration: 9000; easing.type: Easing.InOutSine }
                NumberAnimation { target: wave1; property: "y"; to: 120; duration: 8500; easing.type: Easing.InOutSine }
            }
            ParallelAnimation {
                NumberAnimation { target: wave1; property: "control1Y"; to: 50; duration: 8000; easing.type: Easing.InOutSine }
                NumberAnimation { target: wave1; property: "control2Y"; to: 300; duration: 9000; easing.type: Easing.InOutSine }
                NumberAnimation { target: wave1; property: "y"; to: 180; duration: 8500; easing.type: Easing.InOutSine }
            }
        }
    }
    
    // Front Hill (Primary Accent)
    Shape {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 200
        opacity: 0.9

        ShapePath {
            strokeWidth: 0
            fillColor: Theme.colors.accent
            startX: 0; startY: 200
            PathLine { x: 0; y: 120 }
            PathCubic { 
                id: wave2
                x: root.width; y: 100
                control1X: root.width * 0.4; control1Y: 250
                control2X: root.width * 0.8; control2Y: -20
            }
            PathLine { x: root.width; y: 200 }
            PathLine { x: 0; y: 200 }
        }

        SequentialAnimation {
            running: true
            loops: Animation.Infinite
            ParallelAnimation {
                NumberAnimation { target: wave2; property: "control1Y"; to: -20; duration: 7000; easing.type: Easing.InOutSine }
                NumberAnimation { target: wave2; property: "control2Y"; to: 200; duration: 6500; easing.type: Easing.InOutSine }
                NumberAnimation { target: wave2; property: "y"; to: 160; duration: 7500; easing.type: Easing.InOutSine }
            }
            ParallelAnimation {
                NumberAnimation { target: wave2; property: "control1Y"; to: 250; duration: 7000; easing.type: Easing.InOutSine }
                NumberAnimation { target: wave2; property: "control2Y"; to: -20; duration: 6500; easing.type: Easing.InOutSine }
                NumberAnimation { target: wave2; property: "y"; to: 100; duration: 7500; easing.type: Easing.InOutSine }
            }
        }
    }

    // --- HIGH-END LOGIN CARD (Double-Bezel Architecture) ---
    Rectangle {
        id: loginCard
        anchors.centerIn: parent
        width: 840; height: 540
        radius: Theme.radius.lg
        color: "transparent"

        // Haptic Entry Animation (Scale & Fade)
        scale: 0.95
        opacity: 0.0
        Component.onCompleted: {
            introAnim.start()
        }

        ParallelAnimation {
            id: introAnim
            NumberAnimation { target: loginCard; property: "opacity"; to: 1.0; duration: 1000; easing.type: Easing.OutQuart }
            NumberAnimation { target: loginCard; property: "scale"; to: 1.0; duration: 1200; easing.type: Easing.OutBack; easing.overshoot: 1.1 }
        }

        // 1. Outer Shell (Glassy Bezel)
        Rectangle {
            anchors.fill: parent
            radius: Theme.radius.lg
            color: Theme.colors.surface
            border.width: 1
            border.color: "#CCFFFFFF"
            
            // 2. Inner Core
            Rectangle {
                anchors.fill: parent
                anchors.margins: 10 // The "Double-Bezel" Gap
                radius: Theme.radius.lg - 10
                color: Theme.colors.surfaceDeep
                clip: true

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Left Side: Editorial Branding
                    Rectangle {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 380
                        color: "transparent"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 48
                            spacing: 16

                            // Eyebrow Tag
                            Rectangle {
                                color: Theme.colors.accent
                                radius: 12
                                width: eyebrowText.width + 24
                                height: 24
                                Text {
                                    id: eyebrowText
                                    anchors.centerIn: parent
                                    text: "LINUXCHAT 2.0"
                                    font.family: Theme.fonts.caption
                                    font.pixelSize: 10
                                    font.weight: Font.Bold
                                    font.letterSpacing: 2
                                    color: "#FFFFFF"
                                }
                            }

                            Item { Layout.preferredHeight: 16 }

                            // Massive Typography
                            Text {
                                text: "Breathe.\nConnect.\nThrive."
                                font.family: Theme.fonts.title
                                font.pixelSize: Theme.fonts.titleSize
                                font.weight: Font.Medium
                                color: Theme.colors.text
                                lineHeight: 1.1
                            }
                            
                            Item { Layout.fillHeight: true }

                            Text {
                                text: "Experience the tranquility of secure, real-time communication. Find your oasis."
                                font.family: Theme.fonts.body
                                font.pixelSize: Theme.fonts.bodySize
                                color: Theme.colors.textMuted
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }

                    // Soft Divider
                    Rectangle {
                        Layout.fillHeight: true
                        Layout.topMargin: 64
                        Layout.bottomMargin: 64
                        width: 1
                        color: "#1A064E3B" // Very subtle dark line
                    }

                    // Right Side: Inputs & Actions
                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.centerIn: parent
                            width: 300
                            spacing: Theme.space.md

                            Text { 
                                text: "Enter the Oasis"
                                font.family: Theme.fonts.title
                                font.pixelSize: 28
                                font.weight: Font.Medium
                                color: Theme.colors.text
                            }
                            
                            Item { Layout.preferredHeight: 16 }

                            LCTextField { 
                                id: uf
                                Layout.fillWidth: true
                                placeholderText: "Username" 
                            }
                            
                            LCTextField { 
                                id: pwf
                                Layout.fillWidth: true
                                placeholderText: "Password"
                                echoMode: TextInput.Password
                                Keys.onReturnPressed: doLogin() 
                            }
                            
                            Item { Layout.preferredHeight: 24 }

                            // High-end Nested CTA Button
                            Rectangle {
                                id: loginBtnContainer
                                Layout.fillWidth: true
                                height: 56
                                radius: 28
                                color: Theme.colors.accent
                                
                                property bool isHovered: false
                                property bool isPressed: false
                                
                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onEntered: loginBtnContainer.isHovered = true
                                    onExited: {
                                        loginBtnContainer.isHovered = false
                                        loginBtnContainer.isPressed = false
                                    }
                                    onPressed: loginBtnContainer.isPressed = true
                                    onReleased: {
                                        loginBtnContainer.isPressed = false
                                        doLogin()
                                    }
                                }

                                scale: isPressed ? 0.96 : 1.0
                                Behavior on scale { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }
                                
                                // Color transition on hover
                                Rectangle {
                                    anchors.fill: parent
                                    radius: parent.radius
                                    color: "#000000"
                                    opacity: loginBtnContainer.isHovered ? 0.15 : 0.0
                                    Behavior on opacity { NumberAnimation { duration: 300 } }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 24
                                    anchors.rightMargin: 8
                                    
                                    Text {
                                        Layout.fillWidth: true
                                        text: "Connect"
                                        color: "#FFFFFF"
                                        font.family: Theme.fonts.body
                                        font.pixelSize: 16
                                        font.weight: Font.DemiBold
                                        horizontalAlignment: Text.AlignLeft
                                    }
                                    
                                    // Button-in-Button trailing icon
                                    Rectangle {
                                        width: 40; height: 40; radius: 20
                                        color: "#33FFFFFF"
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: "→"
                                            color: "#FFFFFF"
                                            font.pixelSize: 18
                                            font.weight: Font.Bold
                                        }

                                        // Kinetic hover tension
                                        transform: Translate {
                                            x: loginBtnContainer.isHovered ? 6 : 0
                                            Behavior on x { NumberAnimation { duration: 400; easing.type: Easing.OutBack } }
                                        }
                                    }
                                }
                            }
                            
                            Item { Layout.preferredHeight: 8 }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                Item { Layout.fillWidth: true }
                                Text {
                                    text: "New traveler?"
                                    color: Theme.colors.textMuted
                                    font.family: Theme.fonts.caption
                                }
                                Text {
                                    text: "Create an account"
                                    color: Theme.colors.accent
                                    font.family: Theme.fonts.caption
                                    font.weight: Font.Bold
                                    
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: loginController.registerUser(uf.text.trim(), pwf.text.trim())
                                    }
                                }
                                Item { Layout.fillWidth: true }
                            }

                            Text { 
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: loginController.statusText
                                font.family: Theme.fonts.caption
                                font.pixelSize: Theme.fonts.captionSize
                                color: loginController.isError ? Theme.colors.danger : Theme.colors.accent
                                visible: text.length > 0 
                            }
                        }
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
