import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../styles"

TextField {
    id: control

    property color borderColor: Theme.colors.border
    property color focusColor: Theme.colors.accent
    property color bgColor: "#CC000000" // Deep glass for better contrast against bright images
    property color textColor: Theme.colors.text
    property int radius: Theme.radius.md

    color: textColor
    font.family: Theme.fonts.body
    font.pixelSize: Theme.fonts.bodySize
    placeholderTextColor: Theme.colors.textMuted
    leftPadding: Theme.spacing.md
    rightPadding: Theme.spacing.md
    topPadding: Theme.spacing.sm
    bottomPadding: Theme.spacing.sm

    background: Rectangle {
        implicitWidth: 320
        implicitHeight: 48
        color: control.bgColor
        border.color: control.activeFocus ? control.focusColor : control.borderColor
        border.width: 1
        radius: control.radius

        Behavior on border.color {
            ColorAnimation { duration: 300; easing.type: Easing.OutQuart }
        }
        Behavior on color {
            ColorAnimation { duration: 300; easing.type: Easing.OutQuart }
        }
    }
}
