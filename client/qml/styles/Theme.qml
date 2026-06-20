pragma Singleton
import QtQuick

QtObject {
    id: theme

    // 🌟 High-end Typography (Editorial & Geometric Sans)
    property QtObject fonts: QtObject {
        property string title: "Newsreader"      // Elegant serif for massive headings
        property string body: "LXGW WenKai"      // Soft humanistic readable body
        property string caption: "LXGW WenKai"
        property int titleSize: 48
        property int bodySize: 15
        property int captionSize: 13
    }

    // 🌿 Calibrated "Moving Grassland" Palette (OKLCH derived emeralds/teals)
    property QtObject colors: QtObject {
        property string canvas: "#ECFDF5"        // Very soft mint/cream background (Oasis vibe)
        property string surface: "rgba(255, 255, 255, 0.4)"     // Frosted glass shell
        property string surfaceDeep: "rgba(255, 255, 255, 0.75)"// Inner frosted core
        property string accent: "#059669"        // Primary lush emerald
        property string accentHover: "#047857"   // Deep emerald for interactions
        property string text: "#064E3B"          // Deepest forest green for contrast
        property string textMuted: "#10B981"     // Mid-tone emerald for secondary text
        property string border: "rgba(5, 150, 105, 0.15)" // Subtle organic border
        
        property string danger: "#EF4444"        // Crisp red
        property string success: "#059669"       // Emerald
        
        // Fluid orb colors for the dynamic background
        property string orb1: "#34D399" // Light emerald
        property string orb2: "#A7F3D0" // Very light mint
        property string orb3: "#10B981" // Strong emerald
    }

    // 📐 Spatial Rhythm & Tension (Macro-Whitespace)
    property QtObject space: QtObject {
        property int xs: 4
        property int sm: 8
        property int md: 16
        property int lg: 24
        property int xl: 40
        property int xxl: 64
    }

    // 🟢 Haptic Squircles & Double-Bezel Radii
    property QtObject radius: QtObject {
        property int sm: 8
        property int md: 16
        property int lg: 32     // Huge squircles for high-end feel
        property int xl: 48
        property int full: 999
    }
}
