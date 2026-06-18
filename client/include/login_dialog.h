#pragma once
// login_dialog.h — Login/Register dialog for LinuxChat client
//
// Uses style.qss objectName selectors for all styling.
// Never calls setStyleSheet() inline.
// Newspaper-style background with globe SVG overlay.

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QTimer>

class QPainter;
class ChatClient;

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(ChatClient* client, QWidget* parent = nullptr);

    /// Returns the username entered (valid after accept).
    QString username() const;

    /// Returns the password entered (valid after accept).
    QString password() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void on_connect_clicked();
    void on_login_clicked();
    void on_register_clicked();
    void on_login_ok(const QString& username);
    void on_error_received(const QString& code, const QString& content);
    void on_connected();
    void on_connection_error(const QString& error);

private:
    void setup_ui();
    void set_loading(bool loading);
    void drawNewspaperBackground(QPainter* painter);
    void drawGlobe(QPainter* painter);

    ChatClient*  client_ = nullptr;

    // Server connection
    QLineEdit*   host_edit_     = nullptr;
    QLineEdit*   port_edit_     = nullptr;
    QPushButton* connect_btn_   = nullptr;

    // Credentials
    QLineEdit*   username_edit_ = nullptr;
    QLineEdit*   password_edit_ = nullptr;

    // Action buttons
    QPushButton* login_btn_     = nullptr;
    QPushButton* register_btn_  = nullptr;

    // Status
    QLabel*      status_label_  = nullptr;

    // Connection timeout (prevents UI from hanging indefinitely)
    QTimer*      connect_timer_ = nullptr;
    // Separate timer for app-layer LOGIN/REGISTER response after TCP connected
    QTimer*      login_timer_   = nullptr;
    // Re-entrancy guard: prevents multiple simultaneous connect attempts
    bool         is_connecting_ = false;
};
