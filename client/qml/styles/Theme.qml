import QtQuick

QtObject {
    id: theme

    property string currentSkin: "Minimal"

    // ── Skin data tables ──

    property var _d: {
        "Minimal": {
            canvas: "#FFFFFF", surface: "#FAFAFA", text: "#1A1A1A",
            muted: "#8E8E93", subtle: "#AEAEB2", border: "#E5E5EA",
            accent: "#8E8E93", accentHover: "#636366",
            success: "#34C759", warning: "#F0B232", danger: "#ED4245",
            bubbleSelf: "#F0F0F0", bubbleOther: "transparent",
            bubbleSelfText: "#1A1A1A", bubbleOtherText: "#1A1A1A"
        },
        "Dense": {
            canvas: "#F0F0F5", surface: "#FFFFFF", text: "#1A1A2E",
            muted: "#5A5A72", subtle: "#9292A8", border: "#E2E2EA",
            accent: "#5865F2", accentHover: "#4752C4",
            success: "#23A559", warning: "#F0B232", danger: "#ED4245",
            bubbleSelf: "#DDE3FF", bubbleOther: "#FFFFFF",
            bubbleSelfText: "#1A1A2E", bubbleOtherText: "#1A1A2E"
        },
        "Motion": {
            canvas: "#E8E8EC", surface: "rgba(255,255,255,0.55)", text: "#1C1C1E",
            muted: "#636366", subtle: "#AEAEB2", border: "rgba(255,255,255,0.60)",
            accent: "#4A6CF7", accentHover: "#3B5DE7",
            success: "#34C759", warning: "#F0B232", danger: "#FF3B30",
            bubbleSelf: "rgba(74,108,247,0.88)", bubbleOther: "rgba(255,255,255,0.70)",
            bubbleSelfText: "#FFFFFF", bubbleOtherText: "#1C1C1E"
        },
        "iMessage": {
            canvas: "#FFFFFF", surface: "#FFFFFF", text: "#000000",
            muted: "#8E8E93", subtle: "#AEAEB2", border: "#E5E5EA",
            accent: "#007AFF", accentHover: "#0056CC",
            success: "#34C759", warning: "#FF9500", danger: "#FF3B30",
            bubbleSelf: "#007AFF", bubbleOther: "#E9E9EB",
            bubbleSelfText: "#FFFFFF", bubbleOtherText: "#000000"
        }
    }

    property var _f: {
        "Minimal":  { body: "LXGW WenKai", bodySize: 14, title: "Newsreader", titleSize: 16, mono: "Cascadia Code", monoSize: 13, caption: "LXGW WenKai", captionSize: 11 },
        "Dense":    { body: "LXGW WenKai", bodySize: 12, title: "LXGW WenKai", titleSize: 14, mono: "Cascadia Code", monoSize: 11, caption: "LXGW WenKai", captionSize: 10 },
        "Motion":   { body: "LXGW WenKai", bodySize: 14, title: "Newsreader", titleSize: 18, mono: "Cascadia Code", monoSize: 13, caption: "LXGW WenKai", captionSize: 11 },
        "iMessage": { body: "LXGW WenKai", bodySize: 15, title: "LXGW WenKai", titleSize: 16, mono: "Cascadia Code", monoSize: 13, caption: "LXGW WenKai", captionSize: 11 }
    }

    property var _s: {
        "Minimal":  { xs: 4, sm: 8,  md: 12, lg: 16, xl: 24 },
        "Dense":    { xs: 2, sm: 4,  md: 8,  lg: 12, xl: 16 },
        "Motion":   { xs: 4, sm: 8,  md: 14, lg: 20, xl: 28 },
        "iMessage": { xs: 4, sm: 8,  md: 12, lg: 16, xl: 20 }
    }

    property var _r: {
        "Minimal":  { sm: 4,  md: 8,  lg: 12, full: 9999 },
        "Dense":    { sm: 4,  md: 10, lg: 12, full: 9999 },
        "Motion":   { sm: 8,  md: 14, lg: 16, full: 9999 },
        "iMessage": { sm: 8,  md: 14, lg: 18, full: 9999 }
    }

    property var _b: {
        "Minimal":  { maxWidth: 400, paddingH: 12, paddingV: 14, radius: 4 },
        "Dense":    { maxWidth: 480, paddingH: 10, paddingV: 10, radius: 10 },
        "Motion":   { maxWidth: 420, paddingH: 14, paddingV: 14, radius: 16 },
        "iMessage": { maxWidth: 280, paddingH: 12, paddingV: 10, radius: 18 }
    }

    property var _a: {
        "Minimal":  { size: 28 },
        "Dense":    { size: 36 },
        "Motion":   { size: 44 },
        "iMessage": { size: 40 }
    }

    // ── Token accessors ──

    property var colors: _d[currentSkin] || _d["Minimal"]
    property var fonts:  _f[currentSkin] || _f["Minimal"]
    property var space:  _s[currentSkin] || _s["Minimal"]
    property var radius: _r[currentSkin] || _r["Minimal"]
    property var bubble: _b[currentSkin] || _b["Minimal"]
    property var avatar: _a[currentSkin] || _a["Minimal"]

    function setSkin(name) {
        if (_d[name] !== undefined) {
            currentSkin = name;
        }
    }

    property var skinNames: ["Minimal", "Dense", "Motion", "iMessage"]
}
