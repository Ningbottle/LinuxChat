// MessageBubble.qml — Chat message bubble (3 variants: self / other / system)
//
// PRD §4 Step 3: "3 种变体（self / other / system），参照 chat_view.cpp:create_bubble / create_system_bubble"
// Visual spec: design-a (4px radius, gray accent), design-b (10px, blue), design-c (16px, purple), design-d (18px, iMessage)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LinuxChat

Item {
    id: root

    // ── Public properties (bound by ListView delegate) ──────────
    property string sender:    ""
    property string content:   ""
    property string timestamp: ""
    property bool   isSelf:    false
    property string messageType: "normal"   // "normal" | "system" | "timeSeparator"

    // ── Sizing ──────────────────────────────────────────────────
    implicitWidth:  parent ? parent.width : 400
    implicitHeight: messageType === "system" ? systemBubble.implicitHeight
                   : normalBubble.implicitHeight

    // ════════════════════════════════════════════════════════════
    //  System message (centered, muted pill)
    // ════════════════════════════════════════════════════════════

    Item {
        id: systemBubble
        anchors.horizontalCenter: parent.horizontalCenter
        width:  parent.width
        height: systemLabel.implicitHeight + themeMgr.skin().md * 2
        visible: messageType === "system"

        Rectangle {
            anchors.centerIn: systemLabel
            width:  systemLabel.implicitWidth + themeMgr.skin().lg * 2
            height: systemLabel.implicitHeight + themeMgr.skin().sm * 2
            radius: themeMgr.skin().full
            color:  Qt.rgba(0, 0, 0, 0.05)

            Label {
                id: systemLabel
                anchors.centerIn: parent
                text:           content
                font.family:    themeMgr.skin().caption
                font.pixelSize: themeMgr.skin().captionSize
                color:          themeMgr.skin().muted
            }
        }
    }

    // ════════════════════════════════════════════════════════════
    //  Normal message bubble (self / other)
    // ════════════════════════════════════════════════════════════

    RowLayout {
        id: normalBubble
        anchors.left:   isSelf ? undefined : parent.left
        anchors.right:  isSelf ? parent.right : undefined
        width:          parent.width
        spacing:        themeMgr.skin().sm
        visible:        messageType !== "system"

        // Avatar (other only)
        Rectangle {
            Layout.preferredWidth:  themeMgr.skin().size
            Layout.preferredHeight: themeMgr.skin().size
            Layout.alignment:       Qt.AlignTop
            Layout.leftMargin:      isSelf ? 0 : themeMgr.skin().sm
            Layout.rightMargin:     isSelf ? themeMgr.skin().sm : 0
            visible:                !isSelf
            radius:                 themeMgr.skin().full
            color:                  _avatarColor(sender)

            Label {
                anchors.centerIn: parent
                text:           sender.length > 0 ? sender.charAt(0).toUpperCase() : "?"
                font.family:    themeMgr.skin().body
                font.pixelSize: themeMgr.skin().size * 0.4
                font.bold:      true
                color:          "#FFFFFF"
            }
        }

        // Spacer (self only, pushes bubble right)
        Item { Layout.fillWidth: isSelf; visible: isSelf }

        // Bubble column
        ColumnLayout {
            spacing: 2
            Layout.maximumWidth: themeMgr.skin().maxWidth

            // Sender name (other only, in group chats)
            Label {
                text:           sender
                font.family:    themeMgr.skin().caption
                font.pixelSize: themeMgr.skin().captionSize
                font.bold:      true
                color:          themeMgr.skin().muted
                visible:        !isSelf
            }

            // Bubble rectangle
            Rectangle {
                Layout.fillWidth: true
                radius: themeMgr.skin().radius
                color:  isSelf ? themeMgr.skin().bubbleSelf : themeMgr.skin().bubbleOther
                border.width: isSelf ? 0 : 1
                border.color: themeMgr.skin().border

                ColumnLayout {
                    anchors.fill:    parent
                    anchors.leftMargin:   themeMgr.skin().paddingH
                    anchors.rightMargin:  themeMgr.skin().paddingH
                    anchors.topMargin:    themeMgr.skin().paddingV
                    anchors.bottomMargin: themeMgr.skin().paddingV
                    spacing: 4

                    // Content
                    Label {
                        Layout.fillWidth:  true
                        text:              content
                        font.family:       themeMgr.skin().body
                        font.pixelSize:    themeMgr.skin().bodySize
                        color:             isSelf ? themeMgr.skin().bubbleSelfText : themeMgr.skin().bubbleOtherText
                        wrapMode:          Text.Wrap
                        textFormat:        Text.PlainText
                    }

                    // Timestamp
                    Label {
                        Layout.alignment: Qt.AlignRight
                        text:             _formatTime(timestamp)
                        font.family:      themeMgr.skin().caption
                        font.pixelSize:   10
                        color:            isSelf ? Qt.rgba(1,1,1,0.55) : themeMgr.skin().subtle
                    }
                }
            }
        }

        // Avatar (self only)
        Rectangle {
            Layout.preferredWidth:  themeMgr.skin().size
            Layout.preferredHeight: themeMgr.skin().size
            Layout.alignment:       Qt.AlignTop
            Layout.rightMargin:     themeMgr.skin().sm
            visible:                isSelf
            radius:                 themeMgr.skin().full
            color:                  _avatarColor(sender)

            Label {
                anchors.centerIn: parent
                text:           sender.length > 0 ? sender.charAt(0).toUpperCase() : "?"
                font.family:    themeMgr.skin().body
                font.pixelSize: themeMgr.skin().size * 0.4
                font.bold:      true
                color:          "#FFFFFF"
            }
        }
    }

    // ── Enter animation (PRD: "消息发送气泡：从底部弹入 scale 0.8→1.0 + opacity 0→1, 200ms") ──
    opacity: 0
    scale: 0.8
    y: 6
    Component.onCompleted: bounceIn.start()

    ParallelAnimation {
        id: bounceIn
        NumberAnimation { target: root; property: "opacity"; from: 0.0; to: 1.0; duration: 200; easing.type: Easing.OutCubic }
        NumberAnimation { target: root; property: "scale";   from: 0.8; to: 1.0; duration: 200; easing.type: Easing.OutBack; easing.overshoot: 1.2 }
        NumberAnimation { target: root; property: "y";       from: root.y + 6; to: root.y; duration: 200; easing.type: Easing.OutCubic }
    }

    // ── Helpers ──────────────────────────────────────────────────

    function _formatTime(ts) {
        if (!ts || ts.length === 0) return "";
        return ts;  // Already formatted by C++ backend
    }

    // Deterministic avatar color from username (ref: design-d getColor)
    function _avatarColor(name) {
        var colors = ["#FF6B6B","#4ECDC4","#45B7D1","#96CEB4","#FFEAA7",
                      "#DDA0DD","#98D8C8","#F7DC6F","#BB8FCE","#85C1E9",
                      "#F8C471","#82E0AA","#F1948A","#AED6F1","#D7BDE2",
                      "#A3E4D7"];
        var hash = 0;
        for (var i = 0; i < name.length; i++) {
            hash = ((hash << 5) - hash) + name.charCodeAt(i);
            hash = hash & hash;
        }
        return colors[Math.abs(hash) % colors.length];
    }
}
