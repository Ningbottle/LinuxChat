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
#include <QDebug>
#include <QCoreApplication>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>

// -- Constructor -------------------------------------------------------

MainWindow::MainWindow(ChatClient* client, const QString& username,
                       bool testMode, QWidget* parent)
    : QMainWindow(parent)
    , client_(client)
    , my_username_(username)
    , testMode_(testMode)
{
    qDebug("[MainWindow] Constructor start, testMode=%d", testMode);
    if (testMode_) {
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    }
    setup_ui();
    setup_connections();

    setWindowTitle(QStringLiteral("瓶子交流器 — %1").arg(username));
    setFixedSize(960, 640);
    if (testMode_) {
        if (status_label_) status_label_->setText(QStringLiteral("● 测试模式"));
        if (status_label_) status_label_->setObjectName(QStringLiteral("statusDisconnected")); // reuse muted style
    }
    qDebug("[MainWindow] Constructor complete");
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
    logo_label_->setObjectName(QStringLiteral("topBarTitle"));
    logo_label_->setFont(FontManager::instance().titleFont(16));
    top_bar->addWidget(logo_label_);

    top_bar->addStretch();

    status_label_ = new QLabel(QStringLiteral("● 已连接"));
    status_label_->setObjectName(QStringLiteral("statusConnected"));
    status_label_->setFont(FontManager::instance().bodyFont(11));
    top_bar->addWidget(status_label_);

    // Settings button always (disconnect moved here, with 退出登录)
    settings_btn_ = new QPushButton(QStringLiteral("⚙"));
    settings_btn_->setFixedSize(24, 24);
    settings_btn_->setObjectName(QStringLiteral("settingsBtn"));
    settings_btn_->setFlat(true);
    top_bar->addWidget(settings_btn_);
    connect(settings_btn_, &QPushButton::clicked, this, &MainWindow::on_settings_clicked);

    if (testMode_) {
        // Custom blend min/close for test (融为一体)
        min_btn_ = new QPushButton(QStringLiteral("−"));
        min_btn_->setFixedSize(24, 24);
        min_btn_->setObjectName(QStringLiteral("titleBtn"));
        min_btn_->setFlat(true);
        top_bar->addWidget(min_btn_);

        close_btn_ = new QPushButton(QStringLiteral("×"));
        close_btn_->setFixedSize(24, 24);
        close_btn_->setObjectName(QStringLiteral("titleBtn"));
        close_btn_->setFlat(true);
        top_bar->addWidget(close_btn_);

        connect(min_btn_, &QPushButton::clicked, this, &MainWindow::showMinimized);
        connect(close_btn_, &QPushButton::clicked, this, &QWidget::close);
    }

    main_layout->addLayout(top_bar);

    // -- Separator --
    if (!testMode_) {
        auto* separator = new QFrame;
        separator->setFrameShape(QFrame::HLine);
        separator->setObjectName(QStringLiteral("topSeparator"));
        main_layout->addWidget(separator);
    }

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

    // Disconnect moved to settings (with 退出登录 option)
    // disconnect_btn_ removed from sidebar to avoid ugly UI
    // (logic kept in on_disconnect_clicked for normal mode if needed)

    body_layout->addWidget(sidebar_);

    // -- Chat Area --
    chat_tabs_ = new QTabWidget;
    chat_tabs_->setObjectName(QStringLiteral("chatTabs"));
    chat_tabs_->setDocumentMode(true);

    // Public chat room (always tab 0)
    room_view_ = new ChatView;
    chat_tabs_->addTab(room_view_, QStringLiteral("公共聊天室"));

    body_layout->addWidget(chat_tabs_, 1);

    // Right side globe panel for visibility (narrow, transparent, on right of chat, not interfering content much)
    // Using dedicated panel so globe image is always visible on the right. Widened for recognition.
    QFrame *globePanel = new QFrame;
    globePanel->setFixedWidth(80);  // wider for clear visibility of globe
    globePanel->setObjectName(QStringLiteral("globePanel"));
    QVBoxLayout *gLayout = new QVBoxLayout(globePanel);
    gLayout->setContentsMargins(5, 5, 5, 5);
    gLayout->setSpacing(0);
    QLabel *globeLab = new QLabel;
    QSvgRenderer rend(QStringLiteral(":/images/globe.svg"));
    QPixmap gpm(60, 60);  // larger image
    gpm.fill(Qt::transparent);
    QPainter gp(&gpm);
    rend.render(&gp);
    gp.end();
    globeLab->setPixmap(gpm);
    globeLab->setObjectName(QStringLiteral("globeLabel"));
    gLayout->addStretch();
    gLayout->addWidget(globeLab, 0, Qt::AlignCenter);
    gLayout->addStretch();
    body_layout->addWidget(globePanel);

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

    // Tab change -> clear unread badge
    connect(chat_tabs_, &QTabWidget::currentChanged, this, [this](int index) {
        if (index >= 0) {
            clear_tab_badge(index);
        }
    });

    // Disconnect logic now only in settings dialog
}

// -- Painting -----------------------------------------------------------

void MainWindow::paintEvent(QPaintEvent* event) {
    // Let QSS and base class paint first (white bg per user)
    QMainWindow::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Subtle globe on right (non-interfere chat, low opacity, recognizable)
    // References: high-star Qt custom paint examples (e.g. GTRONICK/QSS, frameless repos); Mimo/Xiaomi cases artistic elements; shuimo.design 水墨 style.
    drawGlobeRight(&painter);
}

void MainWindow::drawWisteriaBackground(QPainter* painter) {
    // legacy kept, unused
}

void MainWindow::drawGlobeRight(QPainter* painter) {
    // Subtle right globe (地球仪) per user + plan: low opacity, recognizable, right of sidebar, non-interfere chat.
    // GitHub high-star searched (GTRONICK/QSS 1.6k, Qt-Advanced-Stylesheets, BreezeStyleSheets custom paint).
    // Mimo/墨墨 ref (shuimo.design 水墨UI, ink lotus painting YouTube/Pinterest): 荷花墨色 for artistic subtle + sidebar.
    static QSvgRenderer renderer(QStringLiteral(":/images/globe.svg"));
    if (!renderer.isValid()) return;

    painter->save();
    painter->setOpacity(0.8);  // make globe clearly visible as pattern on right

    const int sidebarWidth = 240;
    // fixed size 960 per design; for dynamic use rect()
    const int w = 960;
    QRectF target(w - 100, 40, 80, 80);  // small but recognizable on far right
    if (w > sidebarWidth + 100) {
        renderer.render(painter, target);
    }
    painter->restore();
}

// -- ChatClient Signal Handlers -----------------------------------------

void MainWindow::on_broadcast_received(const QString& from, const QString& content,
                                        qint64 timestamp) {
    bool is_self = (from == my_username_);
    room_view_->append_message(from, content, timestamp, is_self);

    // Show unread badge if room tab is not the active tab and message is from others
    if (!is_self) {
        int room_idx = chat_tabs_->indexOf(room_view_);
        if (room_idx >= 0 && chat_tabs_->currentIndex() != room_idx) {
            unread_counts_[room_idx]++;
            update_tab_badge(room_idx);
        }
    }
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
    qDebug("[MainWindow] on_disconnected called");
    online_dot_->setObjectName(QStringLiteral("offlineDot"));
    online_dot_->style()->unpolish(online_dot_);
    online_dot_->style()->polish(online_dot_);

    status_label_->setObjectName(QStringLiteral("statusDisconnected"));
    status_label_->style()->unpolish(status_label_);
    status_label_->style()->polish(status_label_);
    status_label_->setText(QStringLiteral("● 已断开"));

    room_view_->append_system_message(QStringLiteral("与服务器的连接已断开"));

    // Return to login interface as per requirement
    emit returnToLoginRequested();
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

void MainWindow::on_settings_clicked() {
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("设置");
    dlg->setFixedSize(200, 150);
    QVBoxLayout *lay = new QVBoxLayout(dlg);
    lay->addWidget(new QLabel("用户设置"));
    QPushButton *logoutBtn = new QPushButton("退出登录");
    connect(logoutBtn, &QPushButton::clicked, this, [this, dlg]() {
        dlg->accept();
        emit returnToLoginRequested();
    });
    lay->addWidget(logoutBtn);
    QPushButton *closeBtn = new QPushButton("关闭");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    lay->addWidget(closeBtn);
    dlg->exec();
    delete dlg;
}

// -- Test Data (bypass --test-chat / direct QSS iteration) --------------

void MainWindow::populateTestData() {
    if (!room_view_) return;

    // Fill user list (mock)
    if (user_list_) {
        user_list_->clear();
        user_list_->addItem(QStringLiteral("Alice"));
        user_list_->addItem(QStringLiteral("Bob"));
        user_list_->addItem(QStringLiteral("Carol"));
        user_list_->addItem(my_username_);
        user_list_->setCurrentRow(0);  // show the lotus ink selection style
    }

    // Room messages
    room_view_->populateTestData();

    // Create 1-2 private tabs with content
    auto* p1 = get_or_create_private_tab(QStringLiteral("Alice"));
    if (p1) {
        const qint64 ts = QDateTime::currentSecsSinceEpoch() - 120;
        p1->append_message(QStringLiteral("Alice"), QStringLiteral("私聊测试：这个气泡样式怎么样？"), ts, false);
        p1->append_message(my_username_, QStringLiteral("看起来不错，深色 indigo 强调很专业。"), ts + 30, true);
    }

    auto* p2 = get_or_create_private_tab(QStringLiteral("Bob"));
    if (p2) {
        p2->append_message(QStringLiteral("Bob"), QStringLiteral("Bypass 模式下可以直接测试 QSS 了！"), QDateTime::currentSecsSinceEpoch() - 60, false);
    }

    // Live echo wiring for test mode so typing + Send instantly produces self bubble (QSS on #inputField/#sendBtn/#bubbleSelf)
    if (testMode_ && room_view_) {
        connect(room_view_, &ChatView::send_requested, this, [this]() {
            if (!room_view_ || !testMode_) return;
            QString txt = room_view_->take_input();
            if (!txt.trimmed().isEmpty()) {
                room_view_->append_message(my_username_, txt, QDateTime::currentSecsSinceEpoch(), true);
            }
            room_view_->focus_input();
        });
    }
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
    if (testMode_) {
        qDebug("[MainWindow] testMode closeEvent: accepting without logout");
        event->accept();
        return;
    }
    qDebug("[MainWindow] closeEvent triggered, sending LOGOUT");
    client_->send_logout();
    // Brief flush so the LOGOUT frame actually leaves the socket
    // before the process exits and tears down the TCP stack.
    QCoreApplication::processEvents(QEventLoop::AllEvents, 150);
    client_->disconnect_from_server();
    event->accept();
}
