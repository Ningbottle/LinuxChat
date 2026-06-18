#pragma once
// main_window.h -- Main chat window for LinuxChat client
//
// Layout:
//   Top:    Logo bar with connection status
//   Left:   Sidebar (QFrame#sidebar, 240px) -- current user + online user list + disconnect
//   Right:  Chat tabs (QTabWidget#chatTabs) -- public room + private chats
//
// Features: wisteria SVG tiled background, FontManager fonts, 960x640 fixed size.

#include <QMainWindow>
#include <QTabWidget>
#include <QListWidget>
#include <QLabel>
#include <QFrame>
#include <QJsonArray>
#include <QPaintEvent>
#include <QPushButton>

class QPainter;
class ChatClient;
class ChatView;

class MainWindow : public QMainWindow {
    Q_OBJECT

signals:
    void returnToLoginRequested();

public:
    explicit MainWindow(ChatClient* client, const QString& username,
                        bool testMode = false, QWidget* parent = nullptr);

    /// Populate with test/mock data and make interactive for direct QSS testing.
    void populateTestData();

protected:
    void closeEvent(QCloseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    // ChatClient signal handlers
    void on_broadcast_received(const QString& from, const QString& content,
                                qint64 timestamp);
    void on_private_received(const QString& from, const QString& to,
                              const QString& content, qint64 timestamp);
    void on_user_list_updated(const QStringList& users);
    void on_history_received(const QString& target, const QJsonArray& messages);
    void on_notify_received(const QString& content);
    void on_disconnected();

    // UI event handlers
    void on_user_double_clicked(QListWidgetItem* item);
    void on_tab_send_requested();
    void on_room_send_requested();
    void on_settings_clicked();

private:
    void setup_ui();
    void setup_connections();
    void drawWisteriaBackground(QPainter* painter);
    void drawGlobeRight(QPainter* painter);

    /// Get or create a private chat tab for the given username.
    ChatView* get_or_create_private_tab(const QString& username);

    /// Find the ChatView for a given tab index.
    ChatView* chat_view_at(int tab_index) const;

    ChatClient*    client_      = nullptr;
    QString        my_username_;
    bool           testMode_    = false;

    // Top bar
    QLabel*        logo_label_   = nullptr;
    QLabel*        status_label_ = nullptr;
    QPushButton*   settings_btn_ = nullptr;
    QPushButton*   min_btn_      = nullptr;
    QPushButton*   close_btn_    = nullptr;

    // Sidebar
    QFrame*        sidebar_        = nullptr;
    QLabel*        user_label_     = nullptr;
    QLabel*        online_dot_     = nullptr;
    QListWidget*   user_list_      = nullptr;

    // Chat area
    QTabWidget*    chat_tabs_  = nullptr;
    ChatView*      room_view_  = nullptr;  // Public chat room (tab 0)

    // Map: username -> private ChatView
    QMap<QString, ChatView*> private_views_;

    // Unread badge tracking: tab index -> unread count
    QMap<int, int> unread_counts_;

    /// Update the tab label with unread badge for a given tab index.
    void update_tab_badge(int tab_index);

    /// Clear the unread badge for a given tab index.
    void clear_tab_badge(int tab_index);
};
