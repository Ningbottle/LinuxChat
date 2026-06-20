// ChatHeader.qml — Chat area header bar
//
// PRD §4 Step 3: "聊天头部：会话名称 + 在线状态指示器"
// Ref: main_window.cpp:62-112 (top_bar layout)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LinuxChat

Rectangle {
    id: root
    color:      themeMgr.skin().surface
    height:     52

    // ── Public properties ───────────────────────────────────────
    property string sessionName:    qsTr("公共聊天室")
    property bool   isConnected:    chatBackend.connectionStatus === "Connected"
    property bool   sidebarCollapsed: false

    signal settingsClicked()
    signal sidebarToggleClicked()

    RowLayout {
        anchors.fill:       parent
        anchors.leftMargin:  themeMgr.skin().lg
        anchors.rightMargin: themeMgr.skin().md
        spacing:            themeMgr.skin().md

        // Sidebar expand button (visible when collapsed)
        Button {
            id: expandBtn
            Layout.preferredWidth:  28
            Layout.preferredHeight: 28
            flat:    true
            visible: sidebarCollapsed

            contentItem: Label {
                text:           "☰"
                font.pixelSize: 14
                color:          themeMgr.skin().muted
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment:   Text.AlignVCenter
            }
            background: Rectangle {
                radius: themeMgr.skin().sm
                color:  expandBtn.hovered ? Qt.rgba(0,0,0,0.04) : "transparent"
            }
            onClicked: root.sidebarToggleClicked()
        }

        // Session name
        Label {
            Layout.fillWidth: true
            text:             sessionName
            font.family:      themeMgr.skin().title
            font.pixelSize:   themeMgr.skin().titleSize
            font.bold:        true
            color:            themeMgr.skin().text
        }

        // Connection status dot
        Rectangle {
            Layout.preferredWidth:  8
            Layout.preferredHeight: 8
            radius: 4
            color:  isConnected ? themeMgr.skin().success : themeMgr.skin().danger
        }

        // Settings button (PRD: "设置按钮")
        Button {
            id: settingsBtn
            Layout.preferredWidth:  30
            Layout.preferredHeight: 30
            flat: true

            contentItem: Label {
                text:           "⚙"
                font.pixelSize: 16
                color:          settingsBtn.hovered ? themeMgr.skin().text : themeMgr.skin().muted
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment:   Text.AlignVCenter

                Behavior on color { ColorAnimation { duration: 150 } }
            }
            background: Rectangle {
                radius: themeMgr.skin().sm
                color:  settingsBtn.hovered ? Qt.rgba(0,0,0,0.04) : "transparent"
            }
            onClicked: root.settingsClicked()
        }
    }

    // Bottom border
    Rectangle {
        anchors.bottom: parent.bottom
        width:          parent.width
        height:         1
        color:          themeMgr.skin().border
    }
}
