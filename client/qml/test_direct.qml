import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 960; height: 680
    title: "TEST"
    color: "#00FF00"

    Text {
        anchors.centerIn: parent
        text: "GREEN SCREEN TEST"
        font.pixelSize: 50; color: "#FF0000"
    }
}
