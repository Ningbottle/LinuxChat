// login_dialog.cpp — Login/Register dialog implementation
// Newspaper-style background with globe SVG overlay.

#include "login_dialog.h"
#include "chat_client.h"
#include "font_manager.h"

#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QPainter>
#include <QSvgRenderer>
#include <cstdlib>

// ── Constructor ────────────────────────────────────────────────────

LoginDialog::LoginDialog(ChatClient* client, QWidget* parent)
    : QDialog(parent), client_(client)
{
    setup_ui();

    // Connect ChatClient signals
    connect(client_, &ChatClient::connected,
            this, &LoginDialog::on_connected);
    connect(client_, &ChatClient::login_ok,
            this, &LoginDialog::on_login_ok);
    connect(client_, &ChatClient::error_received,
            this, &LoginDialog::on_error_received);
    connect(client_, &ChatClient::connection_error,
            this, &LoginDialog::on_connection_error);
    connect(client_, &ChatClient::disconnected, this, [this]() {
        // If dialog is still visible and user hasn't logged in yet,
        // show a friendly error message
        if (isVisible()) {
            connect_timer_->stop();
            set_loading(false);
            status_label_->setText(QStringLiteral("服务器已断开连接"));
            login_btn_->setEnabled(false);
            register_btn_->setEnabled(false);
        }
    });

    // Connection timeout timer — prevents the UI from hanging
    // indefinitely when the server accepts the TCP handshake but
    // never responds (or the connection is silently dropped).
    connect_timer_ = new QTimer(this);
    connect_timer_->setSingleShot(true);
    connect_timer_->setInterval(10000);  // 10 seconds
    connect(connect_timer_, &QTimer::timeout, this, [this]() {
        if (client_->is_connected()) {
            client_->disconnect_from_server();
        }
        status_label_->setText(QStringLiteral("连接超时，请检查服务器地址和端口"));
        set_loading(false);
        login_btn_->setEnabled(false);
        register_btn_->setEnabled(false);
    });
}

// ── Accessors ──────────────────────────────────────────────────────

QString LoginDialog::username() const {
    return username_edit_->text().trimmed();
}

QString LoginDialog::password() const {
    return password_edit_->text();
}

// ── UI Setup ───────────────────────────────────────────────────────

void LoginDialog::setup_ui() {
    setWindowTitle(QStringLiteral("LinuxChat — 登录"));
    setFixedSize(440, 560);
    setObjectName(QStringLiteral("loginDialog"));

    auto* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(50, 80, 50, 50);
    main_layout->setSpacing(16);

    // Title
    auto* title = new QLabel(QStringLiteral("LinuxChat"));
    title->setObjectName(QStringLiteral("loginTitle"));
    title->setFont(FontManager::instance().titleFont(22));
    title->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(title);

    auto* subtitle = new QLabel(QStringLiteral("网络即时通信工具"));
    subtitle->setObjectName(QStringLiteral("loginSubtitle"));
    subtitle->setFont(FontManager::instance().bodyFont(13));
    subtitle->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(subtitle);

    main_layout->addSpacing(15);

    // Server connection row
    auto* conn_layout = new QHBoxLayout;
    host_edit_ = new QLineEdit;
    host_edit_->setPlaceholderText(QStringLiteral("服务器地址"));
    host_edit_->setText(QStringLiteral("120.55.63.32"));
    host_edit_->setFont(FontManager::instance().bodyFont());
    conn_layout->addWidget(host_edit_, 3);

    port_edit_ = new QLineEdit;
    port_edit_->setPlaceholderText(QStringLiteral("端口"));
    port_edit_->setText(QStringLiteral("8080"));
    port_edit_->setMaximumWidth(80);
    port_edit_->setFont(FontManager::instance().bodyFont());
    conn_layout->addWidget(port_edit_, 1);

    connect_btn_ = new QPushButton(QStringLiteral("连接"));
    connect_btn_->setObjectName(QStringLiteral("secondaryBtn"));
    connect_btn_->setFixedWidth(70);
    connect_btn_->setFont(FontManager::instance().bodyFont(14));
    connect_btn_->setCursor(Qt::PointingHandCursor);
    conn_layout->addWidget(connect_btn_);
    main_layout->addLayout(conn_layout);

    // Separator
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    main_layout->addWidget(sep);

    // Credentials
    username_edit_ = new QLineEdit;
    username_edit_->setPlaceholderText(QStringLiteral("用户名"));
    username_edit_->setFont(FontManager::instance().bodyFont());
    main_layout->addWidget(username_edit_);

    password_edit_ = new QLineEdit;
    password_edit_->setPlaceholderText(QStringLiteral("密码"));
    password_edit_->setEchoMode(QLineEdit::Password);
    password_edit_->setFont(FontManager::instance().bodyFont());
    main_layout->addWidget(password_edit_);

    // Action buttons
    auto* btn_layout = new QHBoxLayout;
    btn_layout->setSpacing(12);

    login_btn_ = new QPushButton(QStringLiteral("登录"));
    login_btn_->setObjectName(QStringLiteral("primaryBtn"));
    login_btn_->setEnabled(false);
    login_btn_->setFont(FontManager::instance().bodyFont(14));
    login_btn_->setCursor(Qt::PointingHandCursor);
    btn_layout->addWidget(login_btn_);

    register_btn_ = new QPushButton(QStringLiteral("注册"));
    register_btn_->setObjectName(QStringLiteral("secondaryBtn"));
    register_btn_->setEnabled(false);
    register_btn_->setFont(FontManager::instance().bodyFont(14));
    register_btn_->setCursor(Qt::PointingHandCursor);
    btn_layout->addWidget(register_btn_);

    main_layout->addLayout(btn_layout);

    // Status label
    status_label_ = new QLabel;
    status_label_->setObjectName(QStringLiteral("loginStatus"));
    status_label_->setAlignment(Qt::AlignCenter);
    status_label_->setFont(FontManager::instance().bodyFont(12));
    main_layout->addWidget(status_label_);

    main_layout->addStretch();

    // Button connections
    connect(connect_btn_, &QPushButton::clicked,
            this, &LoginDialog::on_connect_clicked);
    connect(login_btn_, &QPushButton::clicked,
            this, &LoginDialog::on_login_clicked);
    connect(register_btn_, &QPushButton::clicked,
            this, &LoginDialog::on_register_clicked);

    // Enter key to login
    connect(password_edit_, &QLineEdit::returnPressed,
            this, &LoginDialog::on_login_clicked);
}

// ── Painting ───────────────────────────────────────────────────────

void LoginDialog::paintEvent(QPaintEvent* event) {
    // Let QSS handle the base background (#faf9f7) first
    QDialog::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw newspaper noise texture on top
    drawNewspaperBackground(&painter);

    // Draw globe SVG watermark
    drawGlobe(&painter);
}

void LoginDialog::drawNewspaperBackground(QPainter* painter) {
    // Cache noise pixmap — generated once, reused on all subsequent repaints
    static QPixmap noisePixmap;
    if (noisePixmap.isNull() || noisePixmap.size() != size()) {
        noisePixmap = QPixmap(size());
        noisePixmap.fill(Qt::transparent);
        QPainter p(&noisePixmap);
        p.setPen(QPen(QColor("#d6d3d1"), 1));

        // Fixed seed for deterministic noise (no flicker between repaints)
        std::srand(42);
        for (int i = 0; i < 1000; i++) {
            int x = std::rand() % width();
            int y = std::rand() % height();
            p.drawPoint(x, y);
        }
    }
    painter->drawPixmap(0, 0, noisePixmap);
}

void LoginDialog::drawGlobe(QPainter* painter) {
    // Static renderer — loads SVG from resources once
    static QSvgRenderer renderer(QStringLiteral(":/images/globe.svg"));
    if (renderer.isValid()) {
        painter->setOpacity(0.07);
        QRectF targetRect(20, 20, 400, 400);
        renderer.render(painter, targetRect);
        painter->setOpacity(1.0);
    }
}

// ── Slots ──────────────────────────────────────────────────────────

void LoginDialog::on_connect_clicked() {
    QString host = host_edit_->text().trimmed();
    quint16 port = static_cast<quint16>(port_edit_->text().toUShort());

    if (host.isEmpty() || port == 0) {
        status_label_->setText(QStringLiteral("请输入有效的服务器地址和端口"));
        return;
    }

    status_label_->setText(QStringLiteral("正在连接..."));
    set_loading(true);
    connect_timer_->start();
    client_->connect_to_server(host, port);
}

void LoginDialog::on_login_clicked() {
    if (username().isEmpty() || password().isEmpty()) {
        status_label_->setText(QStringLiteral("请输入用户名和密码"));
        return;
    }
    if (!client_->is_connected()) {
        status_label_->setText(QStringLiteral("请先连接到服务器"));
        return;
    }

    set_loading(true);
    client_->send_login(username(), password());
}

void LoginDialog::on_register_clicked() {
    if (username().isEmpty() || password().isEmpty()) {
        status_label_->setText(QStringLiteral("请输入用户名和密码"));
        return;
    }
    if (!client_->is_connected()) {
        status_label_->setText(QStringLiteral("请先连接到服务器"));
        return;
    }

    set_loading(true);
    client_->send_register(username(), password());
}

void LoginDialog::on_login_ok(const QString& /*username*/) {
    qDebug("[LoginDialog] Login OK, accepting dialog");
    connect_timer_->stop();
    set_loading(false);
    accept(); // Close dialog, proceed to main window
}

void LoginDialog::on_error_received(const QString& code, const QString& content) {
    set_loading(false);
    status_label_->setText(QStringLiteral("[%1] %2").arg(code, content));
}

void LoginDialog::on_connected() {
    connect_timer_->stop();
    set_loading(false);
    status_label_->setText(QStringLiteral("已连接到服务器"));
    login_btn_->setEnabled(true);
    register_btn_->setEnabled(true);
}

void LoginDialog::on_connection_error(const QString& error) {
    connect_timer_->stop();
    set_loading(false);
    status_label_->setText(QStringLiteral("连接失败: %1").arg(error));
    login_btn_->setEnabled(false);
    register_btn_->setEnabled(false);
}

void LoginDialog::set_loading(bool loading) {
    connect_btn_->setEnabled(!loading);
    login_btn_->setEnabled(!loading && client_->is_connected());
    register_btn_->setEnabled(!loading && client_->is_connected());
    username_edit_->setEnabled(!loading);
    password_edit_->setEnabled(!loading);
}
