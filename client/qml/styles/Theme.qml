pragma Singleton
import QtQuick

QtObject {
    id: theme

    // 🌟 High-end Typography (Editorial & Geometric Sans)
    // ---------------------------------------------------------
    // 1. Color Palette (OKLCH-derived Morandi Dark / Ink-Wash)
    // STRICT RULE: Only #AARRGGBB or #RRGGBB. NO rgba() strings.
    // ---------------------------------------------------------
    readonly property QtObject colors: QtObject {
        readonly property color background: "#0E0E0D"    // Deep Ink Charcoal
        readonly property color surface: "#0AFFFFFF"       // 4% Ethereal White Glass
        readonly property color surfaceHover: "#14FFFFFF"  // 8% White Glass
        readonly property color border: "#1AFFFFFF"        // 10% White Border
        readonly property color accent: "#899684"          // Muted Sage / Jade
        readonly property color accentHover: "#9BA995"     // Lighter Sage
        readonly property color text: "#F7F6F2"            // Warm Cream Paper
        readonly property color textMuted: "#8C8A85"       // Soft Warm Gray
        readonly property color error: "#B85C5C"           // Muted Brick Red
    }

    // ---------------------------------------------------------
    // 2. Typography (Variable Serif + Humanist Sans)
    // ---------------------------------------------------------
    readonly property QtObject fonts: QtObject {
        readonly property string display: "Newsreader"
        readonly property string body: "LXGW WenKai"

        readonly property int h1: 48
        readonly property int h2: 32
        readonly property int h3: 24
        readonly property int bodySize: 15
        readonly property int captionSize: 13
    }

    // ---------------------------------------------------------
    // 3. Spacing System (Fibonacci/Structural)
    // ---------------------------------------------------------
    readonly property QtObject spacing: QtObject {
        readonly property int xs: 4
        readonly property int sm: 8
        readonly property int md: 16
        readonly property int lg: 24
        readonly property int xl: 40
        readonly property int xxl: 64
        readonly property int xxxl: 120
    }

    // ---------------------------------------------------------
    // 4. Border Radius (Refined curves)
    // ---------------------------------------------------------
    readonly property QtObject radius: QtObject {
        readonly property int sm: 6
        readonly property int md: 12
        readonly property int lg: 24
        readonly property int xl: 32
        readonly property int full: 9999
    }
}
