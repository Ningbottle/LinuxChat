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
        readonly property color surface: "#15FFFFFF"       // Brighter, very transparent white glass
        readonly property color surfaceHover: "#25FFFFFF"  // Slightly more opaque on hover
        readonly property color border: "#40FFFFFF"        // 25% White Border
        readonly property color accent: "#899684"          // Muted Sage / Jade
        readonly property color accentHover: "#9BA995"     // Lighter Sage
        readonly property color text: "#F7F6F2"            // Warm Cream Paper
        readonly property color textMuted: "#8C8A85"       // Soft Warm Gray
        readonly property color error: "#B85C5C"           // Muted Brick Red

        // Fallbacks for ChatWindow / components
        readonly property color canvas: background
        readonly property color sidebarBg: surface
        readonly property color sidebarHover: surfaceHover
        readonly property color sidebarActive: surfaceHover
        readonly property color success: accent
        readonly property color danger: error
        readonly property color subtle: "#4A4A48"
        readonly property color bubbleSelf: accent
        readonly property color bubbleOther: surface
        readonly property color bubbleSelfText: background
        readonly property color bubbleOtherText: text
    }

    // Add Skin property so Combobox in ChatWindow doesn't break
    property string currentSkin: "Glass"
    property var skinNames: ["Glass"]
    function setSkin(name) { currentSkin = name; }

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
    
    // Alias to satisfy old 'space' references
    readonly property QtObject space: spacing

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

    // ---------------------------------------------------------
    // 5. Bubble System (For Chat UI)
    // ---------------------------------------------------------
    readonly property QtObject bubble: QtObject {
        readonly property int radius: 12
        readonly property int paddingH: 16
        readonly property int paddingV: 10
        readonly property int maxWidth: 480
    }
}
