import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import "../styles"

Item {
    id: root
    
    property string username: ""
    property string fallbackText: username.length > 0 ? username.charAt(0).toUpperCase() : "?"
    property real size: 40
    property bool isActive: false
    
    width: size
    height: size
    
    // Gradient definitions for a premium look
    property var avatarGradients: [
        { stop1: "#F97316", stop2: "#EA580C" }, // Orange
        { stop1: "#8B5CF6", stop2: "#7C3AED" }, // Purple
        { stop1: "#34D399", stop2: "#10B981" }, // Emerald
        { stop1: "#60A5FA", stop2: "#3B82F6" }, // Blue
        { stop1: "#F472B6", stop2: "#EC4899" }, // Pink
        { stop1: "#2DD4BF", stop2: "#14B8A6" }, // Teal
        { stop1: "#FBBF24", stop2: "#F59E0B" }, // Yellow
        { stop1: "#818CF8", stop2: "#6366F1" }  // Indigo
    ]
    
    function getAvatarIndex(name) {
        if (!name || name.length === 0) return 0;
        var hash = 0;
        for (var i = 0; i < name.length; i++) {
            hash = name.charCodeAt(i) + ((hash << 5) - hash);
            hash = hash & hash;
        }
        return Math.abs(hash) % avatarGradients.length;
    }
    
    property int colorIndex: getAvatarIndex(username)
    
    Rectangle {
        id: bgRect
        anchors.fill: parent
        radius: size / 2
        
        gradient: Gradient {
            GradientStop { position: 0.0; color: (!username || username === "") ? "#D1D5DB" : root.avatarGradients[colorIndex].stop1 }
            GradientStop { position: 1.0; color: (!username || username === "") ? "#9CA3AF" : root.avatarGradients[colorIndex].stop2 }
        }
        
        border.color: Theme.colors.border
        border.width: Theme.currentSkin === "Glass" ? 1 : 0
        
        Label {
            anchors.centerIn: parent
            text: root.fallbackText
            color: "#FFFFFF"
            font.family: Theme.fonts.family
            font.pixelSize: root.size * 0.4
            font.weight: Font.Bold
            
            // Text shadow for legibility
            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: "#40000000"
                shadowVerticalOffset: 1
                shadowBlur: 0.2
            }
        }
    }
    
    // Soft inner shadow / border glow using MultiEffect on the circle
    MultiEffect {
        source: bgRect
        anchors.fill: bgRect
        shadowEnabled: true
        shadowColor: "#30000000"
        shadowBlur: 0.5
        shadowVerticalOffset: 2
    }
    
    // Active status indicator
    Rectangle {
        visible: root.isActive
        width: root.size * 0.3
        height: root.size * 0.3
        radius: width / 2
        color: Theme.colors.success
        border.color: Theme.colors.surface
        border.width: 2
        
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: -width * 0.1
        anchors.bottomMargin: -height * 0.1
    }
}
