import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../styles"

TextField {
    id: control

    property color borderColor: Theme.colors.border
    property color focusColor: Theme.colors.accent
    property color bgColor: Theme.colors.surface
    property color textColor: Theme.colors.text
    property int radius: Theme.radius.md

    color: textColor
    font.family: Theme.fonts.body
    font.pixelSize: Theme.fonts.bodySize
    placeholderTextColor: Theme.colors.muted

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 40
        color: control.bgColor
        border.color: control.activeFocus ? control.focusColor : control.borderColor
        border.width: control.activeFocus ? 2 : 1
        radius: control.radius

        Behavior on border.color {
            ColorAnimation { duration: 200 }
        }
    }
}
