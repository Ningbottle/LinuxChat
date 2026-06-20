pragma Singleton
import QtQuick

QtObject {
    id: theme

    property string currentSkin: "Glass"

    // ── Skin data tables ──

    property var _d: {
        "Glass": {
            canvas: "#F4F4F9", surface: "#BFFFFFFF", text: "#1E1E24",
            muted: "#6B7280", subtle: "#9CA3AF", border: "#E6FFFFFF",
            accent: "#4F46E5", accentHover: "#4338CA",
            success: "#10B981", warning: "#F59E0B", danger: "#EF4444",
            bubbleSelf: "#4F46E5", bubbleOther: "#E6FFFFFF",
            bubbleSelfText: "#FFFFFF", bubbleOtherText: "#1E1E24",
            sidebarBg: "#66FFFFFF",
            sidebarActive: "#CCFFFFFF",
            sidebarHover: "#99FFFFFF"
        },
        "Clean": {
            canvas: "#FFFFFF", surface: "#F9FAFB", text: "#111827",
            muted: "#6B7280", subtle: "#9CA3AF", border: "#E5E7EB",
            accent: "#000000", accentHover: "#374151",
            success: "#10B981", warning: "#F59E0B", danger: "#EF4444",
            bubbleSelf: "#111827", bubbleOther: "#F3F4F6",
            bubbleSelfText: "#FFFFFF", bubbleOtherText: "#111827",
            sidebarBg: "#F9FAFB",
            sidebarActive: "#E5E7EB",
            sidebarHover: "#F3F4F6"
        },
        "iMessage": {
            canvas: "#FFFFFF", surface: "#FFFFFF", text: "#000000",
            muted: "#8E8E93", subtle: "#AEAEB2", border: "#E5E5EA",
            accent: "#007AFF", accentHover: "#0056CC",
            success: "#34C759", warning: "#FF9500", danger: "#FF3B30",
            bubbleSelf: "#007AFF", bubbleOther: "#E9E9EB",
            bubbleSelfText: "#FFFFFF", bubbleOtherText: "#000000",
            sidebarBg: "#F2F2F7",
            sidebarActive: "#E5E5EA",
            sidebarHover: "#E5E5EA"
        }
    }

    property var _f: {
        "Glass":    { body: "LXGW WenKai", bodySize: 14, title: "LXGW WenKai", titleSize: 18, mono: "Cascadia Code", monoSize: 13, caption: "LXGW WenKai", captionSize: 11 },
        "Clean":    { body: "LXGW WenKai", bodySize: 14, title: "LXGW WenKai", titleSize: 16, mono: "Cascadia Code", monoSize: 13, caption: "LXGW WenKai", captionSize: 11 },
        "iMessage": { body: "LXGW WenKai", bodySize: 15, title: "LXGW WenKai", titleSize: 16, mono: "Cascadia Code", monoSize: 13, caption: "LXGW WenKai", captionSize: 11 }
    }

    property var _s: {
        "Glass":    { xs: 6, sm: 10, md: 16, lg: 24, xl: 32 },
        "Clean":    { xs: 4, sm: 8,  md: 12, lg: 16, xl: 24 },
        "iMessage": { xs: 4, sm: 8,  md: 12, lg: 16, xl: 20 }
    }

    property var _r: {
        "Glass":    { sm: 8,  md: 16, lg: 24, full: 9999 },
        "Clean":    { sm: 4,  md: 8,  lg: 12, full: 9999 },
        "iMessage": { sm: 8,  md: 14, lg: 18, full: 9999 }
    }

    property var _b: {
        "Glass":    { maxWidth: 420, paddingH: 16, paddingV: 12, radius: 16 },
        "Clean":    { maxWidth: 400, paddingH: 14, paddingV: 12, radius: 12 },
        "iMessage": { maxWidth: 280, paddingH: 14, paddingV: 10, radius: 18 }
    }

    property var _a: {
        "Glass":    { size: 44 },
        "Clean":    { size: 40 },
        "iMessage": { size: 36 }
    }

    // ── Token accessors ──

    property var colors: _d[currentSkin] ? _d[currentSkin] : _d["Glass"]
    property var fonts:  _f[currentSkin] ? _f[currentSkin] : _f["Glass"]
    property var space:  _s[currentSkin] ? _s[currentSkin] : _s["Glass"]
    property var radius: _r[currentSkin] ? _r[currentSkin] : _r["Glass"]
    property var bubble: _b[currentSkin] ? _b[currentSkin] : _b["Glass"]
    property var avatar: _a[currentSkin] ? _a[currentSkin] : _a["Glass"]

    function setSkin(name) {
        if (_d[name] !== undefined) {
            currentSkin = name;
        }
    }

    property var skinNames: ["Glass", "Clean", "iMessage"]
}
