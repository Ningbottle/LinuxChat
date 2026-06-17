// main_window.cpp -- Main chat window implementation
//
// Wisteria SVG tiled background + FontManager fonts + newspaper/retro styling.

#include "main_window.h"
#include "chat_client.h"
#include "chat_view.h"
#include "font_manager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QPainter>
#include <QSvgRenderer>
#include <QTimer>
#include <QCoreApplication>

// -- Constructor -------------------------------------------------------

MainWindow::MainWindow(ChatClient* client, const QString& username,
                       QWidget* parent)
    : QMainWindow(parent)
    , client_(client)
    , my_username_(username)
{
    setup_ui();
    setup_connections();

    setWindowTitle(QStringLiteral("瓶子交流器 — %1").arg(username));
    setFixedSize(960, 640);
}

// -- UI Setup ----------------------------------------------------------

void MainWindow::setup_ui() {
    auto* central = new QWidget;
    central->setObjectName(QStringLiteral("centralWidget"));
    setCentralWidget(central);

    auto* main_layout = new QVBoxLayout(central);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);

    // -- Top Bar --
    auto* top_bar = new QHBoxLayout;
    top_bar->setContentsMargins(16, 12, 16, 12);

    logo_label_ = new QLabel(QStringLiteral("瓶子交流器"));
    logo_label_->setObjectName(QStringLiteral("logoLabel"));
    logo_label_->setFont(FontManager::instance().titleFont(16));
    logo_label_->setStyleSheet(QStringLiteral("color: #1c1917; font-weight: 600;"));
    top_bar->addWidget(logo_label_);

    top_bar->addStretch();

    status_label_ = new QLabel(QStringLiteral("● 已连接"));
    status_label_->setObjectName(QStringLiteral("statusLabel"));
    status_label_->setFont(FontManager::instance().bodyFont(11));
    status_label_->setStyleSheet(QStringLiteral("color: #10b981;"));
    top_bar->addWidget(status_label_);

    main_layout->addLayout(top_bar);

    // -- Separator --
    auto* separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName(QStringLiteral("topSeparator"));
    main_layout->addWidget(separator);

    // -- Body --
    auto* body_layout = new QHBoxLayout;
    body_layout->setContentsMargins(0, 0, 0, 0);
    body_layout->setSpacing(0);

    // -- Sidebar (240px) --
    sidebar_ = new QFrame;
    sidebar_->setObjectName(QStringLiteral("sidebar"));
    sidebar_->setFixedWidth(240);

    auto* sidebar_layout = new QVBoxLayout(sidebar_);
    sidebar_layout->setContentsMargins(8, 8, 8, 8);
    sidebar_layout->setSpacing(8);

    // Current user info
    auto* user_row = new QHBoxLayout;
    online_dot_ = new QLabel;
    online_dot_->setObjectName(QStringLiteral("onlineDot"));
    online_dot_->setFixedSize(8, 8);
    user_row->addWidget(online_dot_);

    user_label_ = new QLabel(my_username_);
    user_label_->setObjectName(QStringLiteral("currentUser"));
    user_label_->setFont(FontManager::instance().bodyFont());
    user_row->addWidget(user_label_, 1);
    sidebar_layout->addLayout(user_row);

    // Section header
    auto* section = new QLabel(QStringLiteral("在线用户"));
    section->setObjectName(QStringLiteral("sectionHeader"));
    section->setFont(FontManager::instance().bodyFont(13));
    sidebar_layout->addWidget(section);

    // User list
    user_list_ = new QListWidget;
    user_list_->setObjectName(QStringLiteral("userList"));
    user_list_->setFont(FontManager::instance().bodyFont());
    sidebar_layout->addWidget(user_list_);

    // Disconnect button
    disconnect_btn_ = new QPushButton(QStringLiteral("断开连接"));
    disconnect_btn_->setObjectName(QStringLiteral("dangerBtn"));
    disconnect_btn_->setFont(FontManager::instance().bodyFont(13));
    disconnect_btn_->setCursor(Qt::PointingHandCursor);
    sidebar_layout->addWidget(disconnect_btn_);

    body_layout->addWidget(sidebar_);

    // -- Chat Area --
    chat_tabs_ = new QTabWidget;
    chat_tabs_->setObjectName(QStringLiteral("chatTabs"));
    chat_tabs_->setDocumentMode(true);

    // Public chat room (always tab 0)
    room_view_ = new ChatView;
    chat_tabs_->addTab(room_view_, QStringLiteral("公共聊天室"));

    body_layout->addWidget(chat_tabs_, 1);

    main_layout->addLayout(body_layout);
}

// -- Signal Connections -------------------------------------------------

void MainWindow::setup_connections() {
    // ChatClient -> MainWindow slots
    connect(client_, &ChatClient::broadcast_received,
            this, &MainWindow::on_broadcast_received);
    connect(client_, &ChatClient::private_received,
            this, &MainWindow::on_private_received);
    connect(client_, &ChatClient::user_list_updated,
            this, &MainWindow::on_user_list_updated);
    connect(client_, &ChatClient::history_received,
            this, &MainWindow::on_history_received);
    connect(client_, &ChatClient::notify_received,
            this, &MainWindow::on_notify_received);
    connect(client_, &ChatClient::disconnected,
            this, &MainWindow::on_disconnected);

    // Room view send
    connect(room_view_, &ChatView::send_requested,
            this, &MainWindow::on_room_send_requested);

    // User list double-click -> open private chat
    connect(user_list_, &QListWidget::itemDoubleClicked,
            this, &MainWindow::on_user_double_clicked);

    // Disconnect button
    connect(disconnect_btn_, &QPushButton::clicked,
            this, &MainWindow::on_disconnect_clicked);
}

// -- Painting -----------------------------------------------------------

void MainWindow::paintEvent(QPaintEvent* event) {
    // Let QSS and base class paint first
    QMainWindow::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw wisteria pattern tiled background
    drawWisteriaBackground(&painter);
}

void MainWindow::drawWisteriaBackground(QPainter* painter) {
    // Static renderer -- loads SVG from resources once, reused on all repaints
    static QSvgRenderer renderer(QStringLiteral(":/images/wisteria-pattern.svg"));
    if (!renderer.isValid()) return;

    painter->setOpacity(0.35);

    // Tile the 300x300 pattern across the entire window
    const QSize patternSize(300, 300);
    for (int x = 0; x < width(); x += patternSize.width()) {
        for (int y = 0; y < height(); y += patternSize.height()) {
            QRectF targetRect(x, y, patternSize.width(), patternSize.height());
            renderer.render(painter, targetRect);
        }
    }

    painter->setOpacity(1.0);
}

// -- ChatClient Signal Handlers -----------------------------------------

void MainWindow::on_broadcast_received(const QString& from, const QString& content,
                                        qint64 timestamp) {
    bool is_self = (from == my_username_);
    room_view_->append_message(from, content, timestamp, is_self);
}

void MainWindow::on_private_received(const QString& from, const QString& to,
                                      const QString& content, qint64 timestamp) {
    // Determine the other party's username
    QString other = (from == my_username_) ? to : from;
    bool is_self = (from == my_username_);

    ChatView* view = get_or_create_private_tab(other);
    view->append_message(from, content, timestamp, is_self);

    // Switch to the private tab if not already there
    int idx = chat_tabs_->indexOf(view);
    if (idx >= 0) {
        chat_tabs_->setCurrentIndex(idx);
    }
}

void MainWindow::on_user_list_updated(const QStringList& users) {
    user_list_->clear();
    for (const QString& user : users) {
        auto* item = new QListWidgetItem(user);
        user_list_->addItem(item);
    }
}

void MainWindow::on_history_received(const QString& target, const QJsonArray& messages) {
    if (target == "__room__") {
        room_view_->load_history(messages, my_username_);
    } else {
        ChatView* view = get_or_create_private_tab(target);
        view->load_history(messages, my_username_);
    }
}

void MainWindow::on_notify_received(const QString& content) {
    room_view_->append_system_message(content);
}

void MainWindow::on_disconnected() {
    online_dot_->setObjectName(QStringLiteral("offlineDot"));
    online_dot_->style()->unpolish(online_dot_);
    online_dot_->style()->polish(online_dot_);

    status_label_->setText(QStringLiteral("● 已断开"));
    status_label_->setStyleSheet(QStringLiteral("color: #a8a29e;"));

    room_view_->append_system_message(QStringLiteral("与服务器的连接已断开"));
}

// -- UI Event Handlers --------------------------------------------------

void MainWindow::on_user_double_clicked(QListWidgetItem* item) {
    QString username = item->text();
    if (username == my_username_) return;

    ChatView* view = get_or_create_private_tab(username);
    int idx = chat_tabs_->indexOf(view);
    if (idx >= 0) {
        chat_tabs_->setCurrentIndex(idx);
        view->focus_input();
    }
}

void MainWindow::on_room_send_requested() {
    QString text = room_view_->take_input();
    if (text.isEmpty()) return;

    client_->send_broadcast(text);
}

void MainWindow::on_tab_send_requested() {
    // Find which tab sent the signal
    auto* view = qobject_cast<ChatView*>(sender());
    if (!view) return;

    // Find the username for this tab
    QString target;
    for (auto it = private_views_.begin(); it != private_views_.end(); ++it) {
        if (it.value() == view) {
            target = it.key();
            break;
        }
    }

    if (target.isEmpty()) return;

    QString text = view->take_input();
    if (text.isEmpty()) return;

    client_->send_private(target, text);
}

void MainWindow::on_disconnect_clicked() {
    client_->send_logout();
    // Give server time to process LOGOUT before closing TCP connection.
    // Without this, disconnect_from_server() kills the socket before
    // the LOGOUT frame has been flushed to the network.
    QTimer::singleShot(200, this, [this]() {
        client_->disconnect_from_server();
    });
}

// -- Helpers ------------------------------------------------------------

ChatView* MainWindow::get_or_create_private_tab(const QString& username) {
    auto it = private_views_.find(username);
    if (it != private_views_.end()) {
        return it.value();
    }

    auto* view = new ChatView;
    chat_tabs_->addTab(view, username);
    private_views_[username] = view;

    // Connect send signal for this private tab
    connect(view, &ChatView::send_requested,
            this, &MainWindow::on_tab_send_requested);

    return view;
}

ChatView* MainWindow::chat_view_at(int tab_index) const {
    return qobject_cast<ChatView*>(chat_tabs_->widget(tab_index));
}

// -- Window Close -------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* event) {
    client_->send_logout();
    // Brief flush so the LOGOUT frame actually leaves the socket
    // before the process exits and tears down the TCP stack.
    QCoreApplication::processEvents(QEventLoop::AllEvents, 150);
    client_->disconnect_from_server();
    event->accept();
}
