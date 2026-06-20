import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import "../styles"

Item {
    id: root

    property string text: ""
    property bool isSelf: false
    property string sender: ""
    property string time: ""
    property bool hasTail: true // Typically based on whether previous/next message is same sender

    // Dynamic width based on contents, constrained by Theme
    width: parent.width
    height: layout.height + Theme.space.sm

    ColumnLayout {
        id: layout
        width: Math.min(implicitWidth, Theme.bubble.maxWidth)
        
        anchors.right: isSelf ? parent.right : undefined
        anchors.left: isSelf ? undefined : parent.left
        anchors.rightMargin: isSelf ? Theme.space.md : 0
        anchors.leftMargin: isSelf ? 0 : Theme.space.md

        // Header (Sender + Time)
        RowLayout {
            Layout.alignment: isSelf ? Qt.AlignRight : Qt.AlignLeft
            visible: !isSelf && root.sender !== ""
            
            Text {
                text: root.sender
                font.family: Theme.fonts.caption
                font.pixelSize: Theme.fonts.captionSize
                color: Theme.colors.subtle
            }
            Text {
                text: root.time
                font.family: Theme.fonts.mono
                font.pixelSize: Theme.fonts.captionSize - 1
                color: Theme.colors.muted
            }
        }

        // The Bubble Container
        Item {
            id: bubbleContainer
            Layout.alignment: isSelf ? Qt.AlignRight : Qt.AlignLeft
            
            implicitWidth: msgText.implicitWidth + Theme.bubble.paddingH * 2
            implicitHeight: msgText.implicitHeight + Theme.bubble.paddingV * 2

            // Main Bubble Background
            Rectangle {
                id: bubbleBg
                anchors.fill: parent
                color: isSelf ? Theme.colors.bubbleSelf : Theme.colors.bubbleOther
                radius: Theme.bubble.radius
                
                border.color: (!isSelf && Theme.currentSkin === "Motion") ? Theme.colors.border : "transparent"
                border.width: border.color !== "transparent" ? 1 : 0
            }

            // Sharp corner for tail
            Rectangle {
                id: sharpCorner
                visible: root.hasTail && (Theme.currentSkin === "iMessage" || Theme.currentSkin === "Minimal")
                width: Theme.bubble.radius
                height: Theme.bubble.radius
                color: bubbleBg.color
                
                anchors.bottom: parent.bottom
                anchors.right: isSelf ? parent.right : undefined
                anchors.left: isSelf ? undefined : parent.left
            }

            // iMessage specific tail swoosh
            Rectangle {
                id: tailSwoosh
                visible: root.hasTail && Theme.currentSkin === "iMessage"
                width: 12
                height: 16
                color: bubbleBg.color
                
                anchors.bottom: parent.bottom
                anchors.right: isSelf ? parent.right : undefined
                anchors.left: isSelf ? undefined : parent.left
                anchors.rightMargin: isSelf ? -8 : 0
                anchors.leftMargin: isSelf ? 0 : -8
                
                // Mask to create the curve
                Rectangle {
                    width: 20
                    height: 20
                    radius: 10
                    color: Theme.colors.canvas
                    anchors.bottom: parent.bottom
                    anchors.right: isSelf ? parent.right : undefined
                    anchors.left: isSelf ? undefined : parent.left
                    anchors.rightMargin: isSelf ? -12 : 0
                    anchors.leftMargin: isSelf ? 0 : -12
                }
            }

            Text {
                id: msgText
                text: root.text
                font.family: Theme.fonts.body
                font.pixelSize: Theme.fonts.bodySize
                color: isSelf ? Theme.colors.bubbleSelfText : Theme.colors.bubbleOtherText
                wrapMode: Text.Wrap
                width: Math.min(implicitWidth, Theme.bubble.maxWidth - Theme.bubble.paddingH * 2)

                anchors.centerIn: parent
            }
        }
        
        // MultiEffect for Shadows (Motion & Minimal)
        MultiEffect {
            source: bubbleContainer
            anchors.fill: bubbleContainer
            shadowEnabled: Theme.currentSkin === "Motion" || Theme.currentSkin === "Minimal"
            shadowColor: "#1A000000" // 10% black
            shadowBlur: 0.5
            shadowVerticalOffset: 2
            shadowHorizontalOffset: 0
            z: -1
        }
        
        // Pop-in animation
        Component.onCompleted: {
            bubbleContainer.scale = 0.8
            bubbleContainer.opacity = 0
            anim.start()
        }
        
        ParallelAnimation {
            id: anim
            NumberAnimation { target: bubbleContainer; property: "scale"; to: 1.0; duration: 200; easing.type: Easing.OutBack }
            NumberAnimation { target: bubbleContainer; property: "opacity"; to: 1.0; duration: 150 }
        }
    }
}
