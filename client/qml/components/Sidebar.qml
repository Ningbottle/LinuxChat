// Sidebar.qml — Left sidebar: user info + session list + online users
//
// PRD §4 Step 3: "侧边栏：当前用户信息 + 在线用户列表"
// Ref: main_window.cpp:128-165 (sidebar_ layout)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LinuxChat

Rectangle {
    id: root
    color:  themeMgr.skin().surface
    border.width: 1
    border.color: themeMgr.skin().border

    // ── Public state ────────────────────────────────────────────
    property string currentTarget: "__room__"
    property bool   collapsed:     false
    property alias  sessionList:   sessionListView
    property alias  userList:      userListView

    signal sessionSelected(string targetName, bool isRoom)
    signal userDoubleClicked(string username)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Current user info ──
        Rectangle {
            Layout.fillWidth:      true
            Layout.preferredHeight: 52
            color: "transparent"

            RowLayout {
                anchors.fill:       parent
                anchors.leftMargin:  themeMgr.skin().md
                anchors.rightMargin: themeMgr.skin().sm
                spacing:            themeMgr.skin().sm

                // Avatar
                Rectangle {
                    Layout.preferredWidth:  32
                    Layout.preferredHeight: 32
                    radius: themeMgr.skin().full
                    color:  themeMgr.skin().accent

                    Label {
                        anchors.centerIn: parent
                        text:           chatBackend.currentUser.length > 0
                                        ? chatBackend.currentUser.charAt(0).toUpperCase() : "?"
                        font.family:    themeMgr.skin().body
                        font.pixelSize: 14
                        font.bold:      true
                        color:          "#FFFFFF"
                    }
                }

                // Username
                Label {
                    Layout.fillWidth: true
                    text:             chatBackend.currentUser
                    font.family:      themeMgr.skin().body
                    font.pixelSize:   themeMgr.skin().bodySize
                    font.bold:        true
                    color:            themeMgr.skin().text
                    elide:            Text.ElideRight
                }

                // Online dot with pulse animation
                // PRD: "在线状态脉冲：绿点 opacity 1.0 ↔ 0.5，1.5s 循环"
                // Ref: design-b-dense.html onlinePulse (scale 0.8-1.2, opacity 0.5-1)
                Rectangle {
                    id: onlineDot
                    Layout.preferredWidth:  10
                    Layout.preferredHeight: 10
                    radius: 5
                    color:  themeMgr.skin().success

                    SequentialAnimation {
                        loops: Animation.Infinite
                        running: chatBackend.connectionStatus === "Connected"

                        ParallelAnimation {
                            NumberAnimation { target: onlineDot; property: "opacity"; to: 0.5; duration: 750; easing.type: Easing.InOutQuad }
                            NumberAnimation { target: onlineDot; property: "scale";   to: 0.8; duration: 750; easing.type: Easing.InOutQuad }
                        }
                        ParallelAnimation {
                            NumberAnimation { target: onlineDot; property: "opacity"; to: 1.0; duration: 750; easing.type: Easing.InOutQuad }
                            NumberAnimation { target: onlineDot; property: "scale";   to: 1.2; duration: 750; easing.type: Easing.InOutQuad }
                        }
                    }
                }
            }
        }

        // Separator
        Rectangle { Layout.fillWidth: true; height: 1; color: themeMgr.skin().border }

        // ── Sessions section ──
        Label {
            Layout.fillWidth:   true
            Layout.leftMargin:  themeMgr.skin().md
            Layout.topMargin:   themeMgr.skin().sm
            Layout.bottomMargin: themeMgr.skin().xs
            text:               qsTr("会话")
            font.family:        themeMgr.skin().caption
            font.pixelSize:     themeMgr.skin().captionSize
            font.bold:          true
            color:              themeMgr.skin().muted
        }

        ListView {
            id: sessionListView
            Layout.fillWidth:  true
            Layout.fillHeight: true
            clip:              true
            model:             chatBackend.sessions

            delegate: SessionTabItem {
                width:          sessionListView.width
                targetName:     model.targetName
                isRoom:         model.isRoom
                unreadCount:    model.unreadCount
                lastMessage:    model.lastMessage
                lastTimestamp:  model.lastTimestamp
                isSelected:     currentTarget === model.targetName

                onClicked: {
                    root.sessionSelected(model.targetName, model.isRoom);
                }
            }

            Label {
                anchors.centerIn: parent
                text:           qsTr("暂无会话")
                font.family:    themeMgr.skin().caption
                font.pixelSize: themeMgr.skin().captionSize
                color:          themeMgr.skin().subtle
                visible:        sessionListView.count === 0
            }
        }

        // Separator
        Rectangle { Layout.fillWidth: true; height: 1; color: themeMgr.skin().border }

        // ── Online users section ──
        Label {
            Layout.fillWidth:   true
            Layout.leftMargin:  themeMgr.skin().md
            Layout.topMargin:   themeMgr.skin().sm
            Layout.bottomMargin: themeMgr.skin().xs
            text:               qsTr("在线 (%1)").arg(chatBackend.onlineUsers.count)
            font.family:        themeMgr.skin().caption
            font.pixelSize:     themeMgr.skin().captionSize
            font.bold:          true
            color:              themeMgr.skin().muted
        }

        ListView {
            id: userListView
            Layout.fillWidth:      true
            Layout.preferredHeight: Math.min(contentHeight, 200)
            Layout.bottomMargin:   themeMgr.skin().sm
            clip:                  true
            model:                 chatBackend.onlineUsers

            delegate: UserListItem {
                width:         userListView.width
                username:      model.username
                isCurrentUser: model.username === chatBackend.currentUser

                MouseArea {
                    anchors.fill:   parent
                    cursorShape:    Qt.PointingHandCursor
                    onDoubleClicked: {
                        if (model.username !== chatBackend.currentUser) {
                            root.userDoubleClicked(model.username);
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                text:           qsTr("暂无在线用户")
                font.family:    themeMgr.skin().caption
                font.pixelSize: themeMgr.skin().captionSize
                color:          themeMgr.skin().subtle
                visible:        userListView.count === 0
            }
        }
    }
}
