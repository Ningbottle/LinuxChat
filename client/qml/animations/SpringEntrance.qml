// SpringEntrance.qml — Login card spring physics entrance
//
// PRD Step 4: "登录界面入场：元素从下方滑入 + 弹簧回弹（SpringAnimation, damping: 0.5）"
// Ref: design-c-motion.html loginFlyIn keyframe
//   (translateY 120px -> -12px -> 5px -> -2px -> 0)

import QtQuick

Item {
    id: root

    // ── Configuration ───────────────────────────────────────────
    property real startScale: 0.9
    property real startOpacity: 0.0
    property real startTranslateY: 60
    property int springDuration: 600
    property real springOvershoot: 1.5

    // ── Apply to target ─────────────────────────────────────────
    property Item target: parent

    Component.onCompleted: springIn.start()

    // ── Animations ──────────────────────────────────────────────

    ParallelAnimation {
        id: springIn

        NumberAnimation {
            target: root.target
            property: "opacity"
            from: root.startOpacity
            to: 1.0
            duration: root.springDuration * 0.6
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: root.target
            property: "scale"
            from: root.startScale
            to: 1.0
            duration: root.springDuration
            easing.type: Easing.OutBack
            easing.overshoot: root.springOvershoot
        }

        NumberAnimation {
            target: root.target
            property: "y"
            from: root.target.y + root.startTranslateY
            to: root.target.y
            duration: root.springDuration
            easing.type: Easing.OutBack
            easing.overshoot: root.springOvershoot
        }
    }

    function start() { springIn.start(); }
}
