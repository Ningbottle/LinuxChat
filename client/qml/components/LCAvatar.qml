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
    
    Rectangle {
        id: bgRect
        anchors.fill: parent
        radius: size / 2
        color: "#f1f5f9" // Lighter background for light theme
        clip: true
        
        border.color: Theme.colors.border
        border.width: Theme.currentSkin === "Glass" ? 1 : 0
        
        Canvas {
            id: canvas
            anchors.fill: parent
            
            // Re-render when username or size changes
            onPaint: {
                var ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);
                
                if (!root.username || root.username === "") {
                    // Fallback empty state
                    return;
                }
                
                // PRNG (sfc32)
                function sfc32(a, b, c, d) {
                    return function() {
                        a >>>= 0; b >>>= 0; c >>>= 0; d >>>= 0; 
                        var t = (a + b) | 0;
                        a = b ^ b >>> 9;
                        b = c + (c << 3) | 0;
                        c = (c << 21 | c >>> 11);
                        d = d + 1 | 0;
                        t = t + d | 0;
                        c = c + t | 0;
                        return (t >>> 0) / 4294967296;
                    }
                }
                
                // Hash (cyrb128)
                function cyrb128(str) {
                    var h1 = 1779033703, h2 = 3144134277,
                        h3 = 1013904242, h4 = 2773480762;
                    for (var i = 0, k; i < str.length; i++) {
                        k = str.charCodeAt(i);
                        h1 = h2 ^ Math.imul(h1 ^ k, 597399067);
                        h2 = h3 ^ Math.imul(h2 ^ k, 2869860233);
                        h3 = h4 ^ Math.imul(h3 ^ k, 951274213);
                        h4 = h1 ^ Math.imul(h4 ^ k, 2716044179);
                    }
                    h1 = Math.imul(h3 ^ (h1 >>> 18), 597399067);
                    h2 = Math.imul(h4 ^ (h2 >>> 22), 2869860233);
                    h3 = Math.imul(h1 ^ (h3 >>> 17), 951274213);
                    h4 = Math.imul(h2 ^ (h4 >>> 19), 2716044179);
                    h1 ^= (h2 ^ h3 ^ h4); h2 ^= h1; h3 ^= h1; h4 ^= h1;
                    return [h1>>>0, h2>>>0, h3>>>0, h4>>>0];
                }
                
                var seedArr = cyrb128(root.username);
                var rand = sfc32(seedArr[0], seedArr[1], seedArr[2], seedArr[3]);
                
                var baseHue = rand() * 360;
                var shapeType = Math.floor(rand() * 3);
                
                // Draw 5 random shapes
                for(var i = 0; i < 5; i++) {
                    var h = (baseHue + (rand() * 60 - 30) + 360) % 360;
                    var s = 0.60 + rand() * 0.30; // 60-90%
                    var l = 0.70 + rand() * 0.25; // 70-95% (light theme)
                    
                    // Convert HSL to a color string
                    ctx.fillStyle = Qt.hsla(h/360, s, l, 0.85);
                    
                    var x = rand() * width;
                    var y = rand() * height;
                    var s_shape = width * (0.3 + rand() * 0.5);
                    
                    if (i % 2 === 0) {
                        ctx.beginPath();
                        ctx.ellipse(x - s_shape/2, y - s_shape/2, s_shape, s_shape);
                        ctx.fill();
                    } else {
                        ctx.save();
                        ctx.translate(x, y);
                        ctx.rotate(rand() * Math.PI * 2);
                        // draw centered rect
                        ctx.fillRect(-s_shape/2, -s_shape/2, s_shape, s_shape);
                        ctx.restore();
                    }
                }
                
                // Draw white stroke overlay
                ctx.strokeStyle = "rgba(255, 255, 255, 0.9)";
                ctx.lineWidth = width * 0.05;
                var cx = width / 2;
                var cy = height / 2;
                
                ctx.beginPath();
                if (shapeType === 0) {
                    ctx.ellipse(cx - width*0.3, cy - height*0.3, width*0.6, height*0.6);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.ellipse(cx - width*0.15, cy - height*0.15, width*0.3, height*0.3);
                } else if (shapeType === 1) {
                    ctx.moveTo(cx - width*0.3, cy);
                    ctx.lineTo(cx + width*0.3, cy);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.moveTo(cx, cy - height*0.3);
                    ctx.lineTo(cx, cy + height*0.3);
                } else {
                    ctx.moveTo(cx, cy - height*0.3);
                    ctx.lineTo(cx - width*0.3, cy + height*0.2);
                    ctx.lineTo(cx + width*0.3, cy + height*0.2);
                    ctx.closePath();
                }
                ctx.stroke();
            }
            
            Connections {
                target: root
                function onUsernameChanged() { canvas.requestPaint(); }
                function onSizeChanged() { canvas.requestPaint(); }
            }
        }
        
        // Only show text fallback if no username (or draw it as overlay?)
        // Let's show it only if empty string.
        Label {
            anchors.centerIn: parent
            text: root.fallbackText
            visible: !root.username || root.username === ""
            color: "#64748B"
            font.family: Theme.fonts.family
            font.pixelSize: root.size * 0.4
            font.weight: Font.Bold
        }
    }
    
    // Soft inner shadow
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
        width: Math.max(10, root.size * 0.3)
        height: Math.max(10, root.size * 0.3)
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
