// InputArea.qml — Message input area with send button
//
// PRD §4 Step 3: "TextArea + 发送按钮，参照 chat_view.cpp:43-75，支持 Ctrl+Enter 发送"
// Ref: chat_view.cpp:43-75 (input_frame layout)

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LinuxChat

Rectangle {
    id: root
    color:      themeMgr.skin().surface
    height:     Math.max(inputRow.implicitHeight + themeMgr.skin().md * 2, 60)

    signal sendMessage(string text)

    RowLayout {
        id: inputRow
        anchors.fill:       parent
        anchors.leftMargin:  themeMgr.skin().lg
        anchors.rightMargin: themeMgr.skin().md
        anchors.topMargin:   themeMgr.skin().sm
        anchors.bottomMargin: themeMgr.skin().sm
        spacing:            themeMgr.skin().sm

        // Input field
        ScrollView {
            Layout.fillWidth:  true
            Layout.preferredHeight: Math.min(inputField.implicitHeight + themeMgr.skin().sm * 2, 100)
            clip: true

            TextArea {
                id: inputField
                placeholderText:     qsTr("输入消息...")
                font.family:         themeMgr.skin().body
                font.pixelSize:      themeMgr.skin().bodySize
                color:               themeMgr.skin().text
                placeholderTextColor: themeMgr.skin().muted
                wrapMode:            TextArea.Wrap
                selectByMouse:       true

                background: Rectangle {
                    radius: themeMgr.skin().md
                    color:  "transparent"
                    border.width: 1
                    border.color: inputField.activeFocus ? themeMgr.skin().accent : themeMgr.skin().border
                    Behavior on border.color { ColorAnimation { duration: 150 } }
                }

                leftPadding:   themeMgr.skin().sm
                rightPadding:  themeMgr.skin().sm
                topPadding:    themeMgr.skin().sm
                bottomPadding: themeMgr.skin().sm

                // Ctrl+Enter to send (PRD: "支持 Ctrl+Enter 发送")
                Keys.onReturnPressed: (event) => {
                    if (event.modifiers & Qt.ControlModifier) {
                        _doSend();
                    } else {
                        event.accepted = false;
                    }
                }
                Keys.onEnterPressed: (event) => {
                    if (event.modifiers & Qt.ControlModifier) {
                        _doSend();
                    } else {
                        event.accepted = false;
                    }
                }
            }
        }

        // Send button
        Button {
            id: sendBtn
            Layout.preferredWidth:  80
            Layout.preferredHeight: 38
            Layout.alignment:       Qt.AlignBottom
            text:     qsTr("发送")
            enabled:  inputField.text.trim().length > 0

            contentItem: Label {
                text:           sendBtn.text
                font.family:    themeMgr.skin().body
                font.pixelSize: themeMgr.skin().bodySize
                font.bold:      true
                color:          sendBtn.enabled ? "#FFFFFF" : themeMgr.skin().muted
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment:   Text.AlignVCenter
            }

            background: Rectangle {
                radius: themeMgr.skin().sm
                color:  sendBtn.enabled ? themeMgr.skin().accent : Qt.rgba(0,0,0,0.05)
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            onClicked: _doSend()
        }
    }

    // Top border
    Rectangle {
        anchors.top: parent.top
        width:       parent.width
        height:      1
        color:       themeMgr.skin().border
    }

    // ── Helpers ──────────────────────────────────────────────────

    function _doSend() {
        var text = inputField.text.trim();
        if (text.length === 0) return;
        root.sendMessage(text);
        inputField.text = "";
        inputField.forceActiveFocus();
    }

    function focusInput() {
        inputField.forceActiveFocus();
    }
}
