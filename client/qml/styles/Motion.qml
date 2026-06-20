import QtQuick

// Motion.qml — "Motion" skin tokens
// Animated, glassmorphism style with semi-transparent surfaces.

QtObject {
    id: motionSkin

    // ── Colors ──
    readonly property var colors: ({
        canvas:   "#F2F2F7",
        surface:  "rgba(255,255,255,0.60)",
        text:     "#1C1C1E",
        muted:    "#8E8E93",
        accent:   "#5856D6",
        success:  "#34C759",
        danger:   "#FF3B30",
        border:   "rgba(255,255,255,0.30)"
    })

    // ── Fonts ──
    readonly property var fonts: ({
        body:      "LXGW WenKai",
        bodySize:  14,
        title:     "Newsreader",
        titleSize: 18,
        code:      "Cascadia Code",
        codeSize:  13
    })

    // ── Spacing ──
    readonly property var spacing: ({
        tiny:   4,
        small:  8,
        medium: 14,
        large:  20,
        xlarge: 28
    })

    // ── Border Radius ──
    readonly property var radius: ({
        small:  8,
        medium: 14,
        large:  18,
        xlarge: 24
    })

    // ── Bubble ──
    readonly property var bubble: ({
        maxWidth: 420,
        radius:   16
    })

    // ── Avatar ──
    readonly property var avatar: ({
        size: 32
    })
}
