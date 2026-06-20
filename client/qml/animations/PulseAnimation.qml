// PulseAnimation.qml — Online status dot pulse
//
// PRD Step 4: "在线状态脉冲：绿点 opacity 1.0 ↔ 0.5，1.5s 循环"
// Ref: design-b-dense.html onlinePulse keyframe
//   (scale 0.8-1.2, opacity 0.5-1)

import QtQuick

Item {
    id: root

    // ── Configuration ───────────────────────────────────────────
    property int pulseDuration: 1500
    property real minOpacity: 0.5
    property real maxOpacity: 1.0
    property real minScale: 0.8
    property real maxScale: 1.2
    property bool running: true

    // ── Apply to target ─────────────────────────────────────────
    property Item target: parent

    SequentialAnimation {
        id: pulseAnim
        loops: Animation.Infinite
        running: root.running

        ParallelAnimation {
            NumberAnimation {
                target: root.target
                property: "opacity"
                from: root.maxOpacity
                to: root.minOpacity
                duration: root.pulseDuration / 2
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: root.target
                property: "scale"
                from: root.maxScale
                to: root.minScale
                duration: root.pulseDuration / 2
                easing.type: Easing.InOutQuad
            }
        }

        ParallelAnimation {
            NumberAnimation {
                target: root.target
                property: "opacity"
                from: root.minOpacity
                to: root.maxOpacity
                duration: root.pulseDuration / 2
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: root.target
                property: "scale"
                from: root.minScale
                to: root.maxScale
                duration: root.pulseDuration / 2
                easing.type: Easing.InOutQuad
            }
        }
    }

    function start() { pulseAnim.start(); }
    function stop()  { pulseAnim.stop();  }
}
