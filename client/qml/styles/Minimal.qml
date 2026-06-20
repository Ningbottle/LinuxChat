import QtQuick

// Minimal.qml — "Minimal" skin tokens
// Clean, professional look matching the original style.qss palette.

QtObject {
    id: minimalSkin

    // ── Colors ──
    readonly property var colors: ({
        canvas:   "#F5F5F5",
        surface:  "#FFFFFF",
        text:     "#1F2937",
        muted:    "#6B7280",
        accent:   "#3B82F6",
        success:  "#10B981",
        danger:   "#EF4444",
        border:   "#E5E7EB"
    })

    // ── Fonts ──
    readonly property var fonts: ({
        body:      "LXGW WenKai",
        bodySize:  14,
        title:     "Newsreader",
        titleSize: 16,
        code:      "Cascadia Code",
        codeSize:  13
    })

    // ── Spacing ──
    readonly property var spacing: ({
        tiny:   4,
        small:  8,
        medium: 12,
        large:  16,
        xlarge: 24
    })

    // ── Border Radius ──
    readonly property var radius: ({
        small:  6,
        medium: 12,
        large:  16,
        xlarge: 20
    })

    // ── Bubble ──
    readonly property var bubble: ({
        maxWidth: 400,
        radius:   12
    })

    // ── Avatar ──
    readonly property var avatar: ({
        size: 28
    })
}
