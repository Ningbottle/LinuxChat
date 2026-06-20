import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import "../styles"

Button {
    id: control

    property color accentColor: Theme.colors.accent
    property color hoverColor: Theme.colors.accentHover
    property color textColor: "#FFFFFF"
    property int radius: Theme.radius.md

    // Mimic the CSS transitions in the HTML buttons
    // e.g. transition: background 0.2s, transform 0.1s;

    contentItem: Text {
        text: control.text
        font.family: Theme.fonts.body
        font.pixelSize: Theme.fonts.bodySize
        font.weight: Font.Medium
        color: control.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Item {
        id: bgContainer
        implicitWidth: 100
        implicitHeight: 40
        
        Rectangle {
            id: bgRect
            anchors.fill: parent
            color: control.pressed ? control.hoverColor : (control.hovered ? control.hoverColor : control.accentColor)
            radius: control.radius

            Behavior on color {
                ColorAnimation { duration: 200 }
            }
        }
        
        // MultiEffect for Drop Shadow
        MultiEffect {
            source: bgRect
            anchors.fill: bgRect
            shadowEnabled: Theme.currentSkin === "Motion" || Theme.currentSkin === "Minimal"
            shadowColor: Theme.currentSkin === "Motion" ? "#26000000" : "#1A000000"
            shadowBlur: control.hovered ? 0.8 : 0.4
            shadowVerticalOffset: control.hovered ? 4 : 2
            shadowHorizontalOffset: 0
            
            Behavior on shadowBlur { NumberAnimation { duration: 200 } }
            Behavior on shadowVerticalOffset { NumberAnimation { duration: 200 } }
        }

        // Slight scale on press
        scale: control.pressed ? 0.98 : 1.0
        Behavior on scale {
            NumberAnimation { duration: 100; easing.type: Easing.OutQuad }
        }
    }
}
