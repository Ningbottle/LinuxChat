// UserListItem.qml — Single user entry in the online users sidebar
//
// Shows a colored avatar circle with initial and the username.
// Highlights if this is the current user.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LinuxChat

Item {
    id: root

    property string username:      ""
    property bool   isCurrentUser: false

    implicitWidth:  200
    implicitHeight: themeMgr.skin().lg + themeMgr.skin().size

    Rectangle {
        anchors.fill:    parent
        anchors.margins: themeMgr.skin().xs
        radius:          themeMgr.skin().sm
        color:           isCurrentUser ? Qt.rgba(0, 0, 0, 0.04) : "transparent"

        RowLayout {
            anchors.fill:        parent
            anchors.leftMargin:  themeMgr.skin().sm
            anchors.rightMargin: themeMgr.skin().sm
            spacing:             themeMgr.skin().sm

            // Avatar circle
            Rectangle {
                Layout.preferredWidth:  themeMgr.skin().size * 0.85
                Layout.preferredHeight: themeMgr.skin().size * 0.85
                radius:                 themeMgr.skin().full
                color:                  isCurrentUser ? themeMgr.skin().accent : Qt.rgba(0, 0, 0, 0.08)

                Label {
                    anchors.centerIn: parent
                    text:           username.length > 0 ? username.charAt(0).toUpperCase() : "?"
                    font.family:    themeMgr.skin().body
                    font.pixelSize: (themeMgr.skin().size * 0.85) * 0.4
                    font.bold:      true
                    color:          isCurrentUser ? "#FFFFFF" : themeMgr.skin().muted
                }
            }

            // Username
            Label {
                Layout.fillWidth:  true
                text:              username
                font.family:       themeMgr.skin().body
                font.pixelSize:    themeMgr.skin().bodySize
                color:             isCurrentUser ? themeMgr.skin().accent : themeMgr.skin().text
                font.bold:         isCurrentUser
                elide:             Text.ElideRight
            }

            // Online dot
            Rectangle {
                Layout.preferredWidth:  8
                Layout.preferredHeight: 8
                radius: 4
                color:  themeMgr.skin().success
            }
        }
    }
}
