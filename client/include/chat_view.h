#pragma once
// chat_view.h — Chat message display and input area
//
// Each ChatView represents one chat tab (public room or private conversation).
// Displays message bubbles using style.qss objectName selectors.

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QJsonArray>

class ChatView : public QWidget {
    Q_OBJECT

public:
    explicit ChatView(QWidget* parent = nullptr);

    /// Append a message bubble to the display.
    /// @param is_self  true if this user sent the message (right-aligned bubble).
    void append_message(const QString& username, const QString& content,
                        qint64 timestamp, bool is_self);

    /// Append a system notification (e.g., "alice joined the chat").
    void append_system_message(const QString& content);

    /// Load history messages (clears existing content first).
    void load_history(const QJsonArray& messages, const QString& my_username);

    /// Get the current input text and clear the input field.
    QString take_input();

    /// Focus the input field.
    void focus_input();

    /// Populate with rich test/mock data for direct QSS testing (no server).
    /// Safe to call once after construction/show. Adds mixed self/other + system messages.
    void populateTestData();

signals:
    /// Emitted when the user clicks Send or presses Enter.
    void send_requested();

private:
    void setup_ui();
    QWidget* create_bubble(const QString& username, const QString& content,
                           qint64 timestamp, bool is_self);
    QWidget* create_system_bubble(const QString& content);
    QWidget* create_time_separator(const QString& time_text);

    QVBoxLayout*  message_layout_ = nullptr;
    QScrollArea*  scroll_area_    = nullptr;
    QWidget*      scroll_widget_  = nullptr;
    QTextEdit*    input_field_    = nullptr;
    QPushButton*  send_btn_       = nullptr;

    qint64 last_message_timestamp_ = 0;  ///< Track last message for time grouping

    /// Format a timestamp into a human-readable Chinese time string
    static QString format_time_separator(qint64 timestamp);
};
