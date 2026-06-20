// TimeSeparator.qml — Time divider between message groups
//
// PRD §4 Step 3: "时间分隔线组件，参照 chat_view.cpp:create_time_separator"
// Ref: chat_view.cpp:241-262

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LinuxChat

Item {
    id: root
    property string timeText: ""

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: 32

    RowLayout {
        anchors.fill:        parent
        anchors.leftMargin:  themeMgr.skin().lg
        anchors.rightMargin: themeMgr.skin().lg
        spacing:             themeMgr.skin().sm

        // Left line
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color:  themeMgr.skin().border
        }

        // Time label
        Label {
            text:           timeText
            font.family:    themeMgr.skin().caption
            font.pixelSize: themeMgr.skin().captionSize
            color:          themeMgr.skin().subtle
        }

        // Right line
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color:  themeMgr.skin().border
        }
    }
}
