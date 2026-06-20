// SettingsPopup.qml — Settings dialog (logout + theme switch)
//
// PRD §4 Step 3: "设置弹窗：退出登录 + 主题切换"
// Ref: main_window.cpp:402-426 (on_settings_clicked)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.settings
import LinuxChat

Popup {
    id: root
    anchors.centerIn: parent
    width:  280
    height: settingsColumn.implicitHeight + 40
    modal:  true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    // ── Signals ─────────────────────────────────────────────────
    signal logoutRequested()

    // ── QSettings persistence (PRD: "皮肤选择持久化") ──────────
    Settings {
        id: skinSettings
        property string skin: "Minimal"
    }

    background: Rectangle {
        radius: themeMgr.skin().lg
        color:  themeMgr.skin().surface
        border.width: 1
        border.color: themeMgr.skin().border
    }

    // Overlay dim
    Overlay.modal: Rectangle {
        color: Qt.rgba(0, 0, 0, 0.3)
    }

    ColumnLayout {
        id: settingsColumn
        anchors.fill:       parent
        anchors.margins:    20
        spacing:            themeMgr.skin().md

        // Title
        Label {
            text:           qsTr("用户设置")
            font.family:    themeMgr.skin().title
            font.pixelSize: themeMgr.skin().titleSize
            font.bold:      true
            color:          themeMgr.skin().text
        }

        // Separator
        Rectangle { Layout.fillWidth: true; height: 1; color: themeMgr.skin().border }

        // ── Theme selector (PRD Step 4: skin selector) ──
        Label {
            text:           qsTr("主题皮肤")
            font.family:    themeMgr.skin().caption
            font.pixelSize: themeMgr.skin().captionSize
            font.bold:      true
            color:          themeMgr.skin().muted
        }

        Repeater {
            model: themeMgr.skinNames()

            delegate: Button {
                Layout.fillWidth:  true
                Layout.preferredHeight: 36
                flat:    true
                text:    modelData
                checked: themeMgr.currentSkin() === modelData

                contentItem: Label {
                    text:           parent.text
                    font.family:    themeMgr.skin().body
                    font.pixelSize: themeMgr.skin().bodySize
                    color:          parent.checked ? themeMgr.skin().accent : themeMgr.skin().text
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment:   Text.AlignVCenter
                    leftPadding:    themeMgr.skin().sm
                }

                background: Rectangle {
                    radius: themeMgr.skin().sm
                    color:  parent.checked ? Qt.rgba(0,0,0,0.05) :
                            parent.hovered ? Qt.rgba(0,0,0,0.03) : "transparent"
                }

                onClicked: {
                    themeMgr.setSkin(modelData);
                    skinSettings.skin = modelData;
                }
            }
        }

        // Separator
        Rectangle { Layout.fillWidth: true; height: 1; color: themeMgr.skin().border }

        // Logout button (PRD: "退出登录按钮")
        Button {
            id: logoutBtn
            Layout.fillWidth:  true
            Layout.preferredHeight: 40
            text:     qsTr("退出登录")

            contentItem: Label {
                text:           logoutBtn.text
                font.family:    themeMgr.skin().body
                font.pixelSize: themeMgr.skin().bodySize
                font.bold:      true
                color:          logoutBtn.hovered ? "#FFFFFF" : themeMgr.skin().danger
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment:   Text.AlignVCenter

                Behavior on color { ColorAnimation { duration: 150 } }
            }

            background: Rectangle {
                radius: themeMgr.skin().sm
                color:  logoutBtn.hovered ? themeMgr.skin().danger : Qt.rgba(0,0,0,0.03)
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            onClicked: {
                root.close();
                root.logoutRequested();
            }
        }

        // Close button
        Button {
            id: closeBtn
            Layout.fillWidth:  true
            Layout.preferredHeight: 36
            text:     qsTr("关闭")

            contentItem: Label {
                text:           closeBtn.text
                font.family:    themeMgr.skin().body
                font.pixelSize: themeMgr.skin().bodySize
                color:          themeMgr.skin().muted
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment:   Text.AlignVCenter
            }

            background: Rectangle {
                radius: themeMgr.skin().sm
                color:  closeBtn.hovered ? Qt.rgba(0,0,0,0.04) : "transparent"
            }

            onClicked: root.close()
        }
    }
}
