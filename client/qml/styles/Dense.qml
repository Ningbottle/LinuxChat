import QtQuick

// Dense.qml — "Dense" skin tokens
// Compact, Discord-like layout with tighter spacing and smaller fonts.

QtObject {
    id: denseSkin

    // ── Colors ──
    readonly property var colors: ({
        canvas:   "#F0F0F5",
        surface:  "#E8E8ED",
        text:     "#1C1C1E",
        muted:    "#8E8E93",
        accent:   "#5856D6",
        success:  "#34C759",
        danger:   "#FF3B30",
        border:   "#D1D1D6"
    })

    // ── Fonts ──
    readonly property var fonts: ({
        body:      "LXGW WenKai",
        bodySize:  12,
        title:     "LXGW WenKai",
        titleSize: 14,
        code:      "Cascadia Code",
        codeSize:  11
    })

    // ── Spacing ──
    readonly property var spacing: ({
        tiny:   2,
        small:  4,
        medium: 8,
        large:  12,
        xlarge: 16
    })

    // ── Border Radius ──
    readonly property var radius: ({
        small:  4,
        medium: 8,
        large:  12,
        xlarge: 16
    })

    // ── Bubble ──
    readonly property var bubble: ({
        maxWidth: 480,
        radius:   8
    })

    // ── Avatar ──
    readonly property var avatar: ({
        size: 24
    })
}
