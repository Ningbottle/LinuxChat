import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../styles"
import "../components"

Rectangle {
    id: root
    color: Theme.colors.background
    clip: true

    // --- MAJESTIC INK-WASH GRASSLAND BACKGROUND ---
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
            opacity: 0.8
            
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

        // Minimal Dark / Film Grain Noise overlay
        Rectangle {
            anchors.fill: parent
            color: "#05000000" // Extremely subtle vignette/darkening
        }
    }

    // --- HIGH-END LOGIN CARD (Ethereal Frosted Glass) ---
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
            color: Qt.rgba(20/255, 20/255, 20/255, 0.3) // More transparent and brighter than solid gray
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.1)
            
            // 2. Inner Core
            Rectangle {
                anchors.fill: parent
                anchors.margins: 10 // The "Double-Bezel" Gap
                radius: Theme.radius.lg - 10
                color: Qt.rgba(40/255, 40/255, 40/255, 0.35) // More transparent inner core
                clip: true

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Left Side: Editorial Branding
                    Rectangle {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 420
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
                                    font.family: Theme.fonts.body
                                    font.pixelSize: 10
                                    font.weight: Font.Bold
                                    font.letterSpacing: 2
                                    color: "#121110"
                                }
                            }

                            Item { Layout.preferredHeight: 16 }

                            // Massive Typography
                            Text {
                                id: poemText
                                text: ""
                                font.family: "LXGW WenKai"
                                font.pixelSize: 30 // Slightly smaller to fit better
                                font.weight: Font.Medium
                                color: Theme.colors.text
                                lineHeight: 1.4
                                width: parent.width - 40 // More padding on the right to prevent overlapping the line
                                wrapMode: Text.WordWrap
                                
                                Component.onCompleted: {
                                    var poems = [
                                        "行到水穷处\n坐看云起时",
                                        "大漠孤烟直\n长河落日圆",
                                        "明月松间照\n清泉石上流",
                                        "海上生明月\n天涯共此时",
                                        "落霞与孤鹜齐飞\n秋水共长天一色",
                                        "会当凌绝顶\n一览众山小"
                                    ];
                                    poemText.text = poems[Math.floor(Math.random() * poems.length)];
                                }
                            }
                            
                            Item { Layout.fillHeight: true }

                            Text {
                                text: "Experience the tranquility of secure, real-time communication.\nFind your peace."
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
                        color: Theme.colors.border
                    }

                    // Right Side: Inputs & Actions
                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.centerIn: parent
                            width: 320
                            spacing: 8 // Reduced spacing to fit everything vertically

                            Text { 
                                text: "Enter the Void"
                                font.family: Theme.fonts.display
                                font.pixelSize: 28
                                font.weight: Font.Medium
                                color: Theme.colors.text
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                LCTextField { 
                                    id: hostField
                                    Layout.fillWidth: true
                                    placeholderText: "Server IP" 
                                    text: "120.55.63.32"
                                }
                                LCTextField { 
                                    id: portField
                                    Layout.preferredWidth: 80
                                    placeholderText: "Port" 
                                    text: "18080"
                                }
                            }

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

                            // High-end Nested CTA Button
                            Rectangle {
                                id: loginBtnContainer
                                Layout.fillWidth: true
                                Layout.preferredHeight: 48
                                radius: 24
                                color: Theme.colors.text
                                
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
                                    opacity: loginBtnContainer.isHovered ? 0.05 : 0.0
                                    Behavior on opacity { NumberAnimation { duration: 300 } }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 24
                                    anchors.rightMargin: 8
                                    
                                    Text {
                                        Layout.fillWidth: true
                                        text: "Connect"
                                        color: Theme.colors.background
                                        font.family: Theme.fonts.body
                                        font.pixelSize: 16
                                        font.weight: Font.DemiBold
                                        horizontalAlignment: Text.AlignLeft
                                    }
                                    
                                    // Button-in-Button trailing icon
                                    Rectangle {
                                        width: 40; height: 40; radius: 20
                                        color: Theme.colors.background
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: "→"
                                            color: Theme.colors.text
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

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                Item { Layout.fillWidth: true }
                                Text {
                                    text: "New traveler?"
                                    color: Theme.colors.textMuted
                                    font.family: Theme.fonts.body
                                    font.pixelSize: 15
                                }
                                Text {
                                    text: "Create an account"
                                    color: Theme.colors.accent
                                    font.family: Theme.fonts.body
                                    font.pixelSize: 15
                                    font.weight: Font.Bold
                                    
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: loginController.registerUser(uf.text.trim(), pwf.text.trim())
                                    }
                                }
                                Item { Layout.fillWidth: true }
                            }

                            Item { Layout.preferredHeight: 8 }

                            // High-end Test Connection Button
                            Rectangle {
                                id: testBtnContainer
                                Layout.fillWidth: true
                                Layout.preferredHeight: 44 // Reduced height from 56 so it's not cut off
                                radius: 28
                                color: testBtnContainer.isHovered ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, 0.05) // Brighter background
                                border.color: Theme.colors.border
                                border.width: 1
                                
                                property bool isHovered: false
                                property bool isPressed: false
                                
                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onEntered: testBtnContainer.isHovered = true
                                    onExited: {
                                        testBtnContainer.isHovered = false
                                        testBtnContainer.isPressed = false
                                    }
                                    onPressed: testBtnContainer.isPressed = true
                                    onReleased: {
                                        testBtnContainer.isPressed = false
                                        var host = hostField.text.trim();
                                        var port = parseInt(portField.text.trim());
                                        if (host.length > 0 && port > 0) {
                                            loginController.connectToServer(host, port);
                                        }
                                    }
                                }

                                scale: testBtnContainer.isPressed ? 0.97 : 1.0
                                Behavior on scale { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }

                                RowLayout {
                                    anchors.centerIn: parent
                                    spacing: 12
                                    
                                    Text {
                                        text: "Server Status:"
                                        color: Theme.colors.textMuted
                                        font.family: Theme.fonts.body
                                        font.pixelSize: 16 // Increased legibility
                                    }
                                    
                                    Text {
                                        text: "Test Connection"
                                        color: Theme.colors.text
                                        font.family: Theme.fonts.body
                                        font.pixelSize: 18
                                        font.weight: Font.Bold
                                    }
                                }
                            }

                            Text { 
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: loginController.statusText
                                font.family: Theme.fonts.body
                                font.pixelSize: 16
                                color: loginController.isError ? Theme.colors.error : Theme.colors.accent
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
