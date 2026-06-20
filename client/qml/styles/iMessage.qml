import QtQuick

// iMessage.qml — "iMessage" skin tokens
// Apple Messages aesthetic with tight bubble widths and rounded corners.

QtObject {
    id: iMessageSkin

    // ── Colors ──
    readonly property var colors: ({
        canvas:   "#FFFFFF",
        surface:  "#FFFFFF",
        text:     "#000000",
        muted:    "#8E8E93",
        accent:   "#007AFF",
        success:  "#34C759",
        danger:   "#FF3B30",
        border:   "#E5E5EA"
    })

    // ── Fonts ──
    readonly property var fonts: ({
        body:      "LXGW WenKai",
        bodySize:  15,
        title:     "LXGW WenKai",
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
        xlarge: 20
    })

    // ── Border Radius ──
    readonly property var radius: ({
        small:  8,
        medium: 14,
        large:  18,
        xlarge: 20
    })

    // ── Bubble ──
    readonly property var bubble: ({
        maxWidth: 280,
        radius:   18
    })

    // ── Avatar ──
    readonly property var avatar: ({
        size: 28
    })
}
