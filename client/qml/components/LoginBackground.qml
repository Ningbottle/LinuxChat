import QtQuick

Item {
    id: root

    // Subtle noise texture overlay
    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d");
            // Draw random dots for newspaper texture
            // Use deterministic seed (fixed positions)
            ctx.fillStyle = "rgba(200, 200, 200, 0.15)";
            for (var i = 0; i < 800; i++) {
                var x = (i * 7919) % width;  // deterministic pseudo-random
                var y = (i * 6271) % height;
                ctx.fillRect(x, y, 1, 1);
            }
        }
    }

    // Globe watermark (using the SVG from resources)
    Image {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        width: 300
        height: 300
        source: "qrc:/images/globe.svg"
        opacity: 0.06
        fillMode: Image.PreserveAspectFit
    }
}
