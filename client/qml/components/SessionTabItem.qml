// SessionTabItem.qml — Session entry in the sidebar session list
//
// Displays session name, last message preview, timestamp, and unread badge.
// Used as delegate for the sessions ListView in Sidebar.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LinuxChat

Item {
    id: root

    property string targetName:    ""
    property bool   isRoom:        true
    property int    unreadCount:   0
    property string lastMessage:   ""
    property string lastTimestamp: ""
    property bool   isSelected:    false

    implicitWidth:  200
    implicitHeight: themeMgr.skin().lg * 2 + 8

    Rectangle {
        anchors.fill:    parent
        anchors.margins: themeMgr.skin().xs
        radius:          themeMgr.skin().sm
        color:           isSelected ? Qt.rgba(0, 0, 0, 0.06)
                       : mouseArea.containsMouse ? Qt.rgba(0, 0, 0, 0.03)
                       : "transparent"

        Behavior on color { ColorAnimation { duration: 120 } }

        MouseArea {
            id:           mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape:  Qt.PointingHandCursor
            onClicked:    root.clicked()
        }

        RowLayout {
            anchors.fill:        parent
            anchors.leftMargin:  themeMgr.skin().sm
            anchors.rightMargin: themeMgr.skin().sm
            spacing:             themeMgr.skin().sm

            // Icon / avatar
            Rectangle {
                Layout.preferredWidth:  themeMgr.skin().size * 0.85
                Layout.preferredHeight: themeMgr.skin().size * 0.85
                radius:                 isRoom ? themeMgr.skin().sm : themeMgr.skin().full
                color:                  isRoom ? themeMgr.skin().accent : Qt.rgba(0, 0, 0, 0.08)

                Label {
                    anchors.centerIn: parent
                    text:           isRoom ? "💬" : (targetName.length > 0 ? targetName.charAt(0).toUpperCase() : "?")
                    font.family:    themeMgr.skin().body
                    font.pixelSize: (themeMgr.skin().size * 0.85) * 0.4
                    font.bold:      true
                    color:          isRoom ? "#FFFFFF" : themeMgr.skin().muted
                }
            }

            // Text column
            ColumnLayout {
                Layout.fillWidth: true
                spacing:         1

                Label {
                    Layout.fillWidth: true
                    text:             isRoom ? qsTr("公共聊天室") : targetName
                    font.family:      themeMgr.skin().body
                    font.pixelSize:   themeMgr.skin().bodySize
                    font.bold:        unreadCount > 0
                    color:            themeMgr.skin().text
                    elide:            Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text:             lastMessage.length > 0 ? lastMessage : ""
                    font.family:      themeMgr.skin().caption
                    font.pixelSize:   themeMgr.skin().captionSize
                    color:            themeMgr.skin().muted
                    elide:            Text.ElideRight
                    visible:          lastMessage.length > 0
                    maximumLineCount: 1
                }
            }

            // Right column: time + badge
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                spacing:         4

                Label {
                    text:           lastTimestamp
                    font.family:    themeMgr.skin().caption
                    font.pixelSize: 10
                    color:          themeMgr.skin().subtle
                    visible:        lastTimestamp.length > 0
                    Layout.alignment: Qt.AlignRight
                }

                // Unread badge (PRD: "未读消息标记")
                Rectangle {
                    Layout.alignment:      Qt.AlignRight
                    Layout.preferredWidth:  Math.max(badgeLabel.implicitWidth + 8, 20)
                    Layout.preferredHeight: 18
                    radius: 9
                    color:  themeMgr.skin().danger
                    visible: unreadCount > 0

                    Label {
                        id: badgeLabel
                        anchors.centerIn: parent
                        text:           unreadCount > 99 ? "99+" : unreadCount.toString()
                        font.family:    themeMgr.skin().caption
                        font.pixelSize: 10
                        font.bold:      true
                        color:          "#FFFFFF"
                    }
                }
            }
        }
    }

    signal clicked()
}
