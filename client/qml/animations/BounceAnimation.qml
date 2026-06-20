// BounceAnimation.qml — Message bubble entrance animation
//
// PRD Step 4: "消息发送气泡：从底部弹入（scale 0.8 → 1.0 + opacity 0 → 1, 200ms）"
// Ref: design-c-motion.html messageSlideUp keyframe (spring overshoot)

import QtQuick

Item {
    id: root

    // ── Configuration ───────────────────────────────────────────
    property int duration: 200
    property real startScale: 0.8
    property real startOpacity: 0.0
    property real startY: 8

    // ── Apply to target item ────────────────────────────────────
    // Usage: BounceAnimation { target: myBubble }
    property Item target: parent

    // Trigger on Component.onCompleted of the target
    Connections {
        target: root.target
        function onWidthChanged() { _startIfReady(); }
    }

    Component.onCompleted: _startIfReady()

    function _startIfReady() {
        if (target && target.visible) {
            bounceIn.start();
        }
    }

    // ── Animations ──────────────────────────────────────────────

    ParallelAnimation {
        id: bounceIn

        NumberAnimation {
            target: root.target
            property: "opacity"
            from: root.startOpacity
            to: 1.0
            duration: root.duration
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: root.target
            property: "scale"
            from: root.startScale
            to: 1.0
            duration: root.duration
            easing.type: Easing.OutBack
            easing.overshoot: 1.2
        }

        NumberAnimation {
            target: root.target
            property: "y"
            from: root.target.y + root.startY
            to: root.target.y
            duration: root.duration
            easing.type: Easing.OutCubic
        }
    }

    function start() { bounceIn.start(); }
}
