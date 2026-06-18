// chat_view.cpp — Chat message display and input area implementation

#include "chat_view.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>

// ── Constructor ────────────────────────────────────────────────────

ChatView::ChatView(QWidget* parent)
    : QWidget(parent)
{
    setup_ui();
}

// ── UI Setup ───────────────────────────────────────────────────────

void ChatView::setup_ui() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // Scrollable message area
    scroll_area_ = new QScrollArea;
    scroll_area_->setObjectName(QStringLiteral("messageArea"));
    scroll_area_->setWidgetResizable(true);
    scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area_->setFrameShape(QFrame::NoFrame);

    scroll_widget_ = new QWidget;
    scroll_widget_->setObjectName(QStringLiteral("messageContainer"));

    message_layout_ = new QVBoxLayout(scroll_widget_);
    message_layout_->setContentsMargins(20, 12, 20, 12);  // per design spec for chat area
    message_layout_->setSpacing(8);
    message_layout_->addStretch(); // Push messages to top

    scroll_area_->setWidget(scroll_widget_);
    outer->addWidget(scroll_area_, 1);

    // Input area (wrap for #inputArea round container 圆润感)
    auto* input_frame = new QFrame;
    input_frame->setObjectName(QStringLiteral("inputArea"));
    auto* input_layout = new QHBoxLayout(input_frame);
    input_layout->setContentsMargins(12, 8, 12, 12);
    input_layout->setSpacing(8);

    input_field_ = new QTextEdit;
    input_field_->setObjectName(QStringLiteral("inputField"));
    input_field_->setPlaceholderText(QStringLiteral("输入消息..."));
    input_field_->setMaximumHeight(80);
    input_field_->setMinimumHeight(40);
    input_field_->setTabChangesFocus(true);
    input_layout->addWidget(input_field_, 1);

    send_btn_ = new QPushButton(QStringLiteral("发送"));
    send_btn_->setObjectName(QStringLiteral("sendBtn"));
    send_btn_->setFixedWidth(80);
    send_btn_->setFixedHeight(36);
    input_layout->addWidget(send_btn_);

    outer->addWidget(input_frame);

    // Connect send button
    connect(send_btn_, &QPushButton::clicked, this, &ChatView::send_requested);

    // Ctrl+Enter to send
    connect(input_field_, &QTextEdit::textChanged, this, [this]() {
        // Auto-resize input field (min 40, max 80) - robust after send/clear
        int doc_height = input_field_->document()->size().height();
        int target = qBound(40, doc_height + 12, 80);
        input_field_->setFixedHeight(target);
    });
}

// ── Public API ─────────────────────────────────────────────────────

QString ChatView::format_time_separator(qint64 timestamp) {
    QDateTime msg_dt = QDateTime::fromSecsSinceEpoch(timestamp);
    QDateTime now = QDateTime::currentDateTime();
    QDate msg_date = msg_dt.date();
    QDate today = now.date();

    if (msg_date == today) {
        return QStringLiteral("今天 %1").arg(msg_dt.toString(QStringLiteral("HH:mm")));
    } else if (msg_date == today.addDays(-1)) {
        return QStringLiteral("昨天 %1").arg(msg_dt.toString(QStringLiteral("HH:mm")));
    } else {
        return msg_dt.toString(QStringLiteral("yyyy-MM-dd HH:mm"));
    }
}

void ChatView::append_message(const QString& username, const QString& content,
                              qint64 timestamp, bool is_self) {
    // Time grouping: insert separator if interval > 5 minutes (300 seconds)
    if (last_message_timestamp_ > 0 && (timestamp - last_message_timestamp_) > 300) {
        QWidget* separator = create_time_separator(format_time_separator(timestamp));
        int insert_pos = message_layout_->count() - 1;
        message_layout_->insertWidget(insert_pos, separator);
    }
    last_message_timestamp_ = timestamp;

    QWidget* bubble = create_bubble(username, content, timestamp, is_self);

    // Insert before the stretch
    int insert_pos = message_layout_->count() - 1;
    message_layout_->insertWidget(insert_pos, bubble);

    // Auto-scroll to bottom
    QTimer::singleShot(0, this, [this]() {
        scroll_area_->verticalScrollBar()->setValue(
            scroll_area_->verticalScrollBar()->maximum());
    });
}

void ChatView::append_system_message(const QString& content) {
    QWidget* bubble = create_system_bubble(content);
    int insert_pos = message_layout_->count() - 1;
    message_layout_->insertWidget(insert_pos, bubble);

    QTimer::singleShot(0, this, [this]() {
        scroll_area_->verticalScrollBar()->setValue(
            scroll_area_->verticalScrollBar()->maximum());
    });
}

void ChatView::load_history(const QJsonArray& messages, const QString& my_username) {
    // Remove all existing message widgets (keep the stretch at end)
    while (message_layout_->count() > 1) {
        QLayoutItem* item = message_layout_->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Reset time grouping state
    last_message_timestamp_ = 0;

    // Add history messages
    for (const auto& val : messages) {
        QJsonObject obj = val.toObject();
        QString from    = obj["from"].toString();
        QString content = obj["content"].toString();
        qint64 ts       = static_cast<qint64>(obj["timestamp"].toDouble());
        bool is_self    = (from == my_username);
        append_message(from, content, ts, is_self);
    }
}

QString ChatView::take_input() {
    QString text = input_field_->toPlainText().trimmed();
    if (!text.isEmpty()) {
        input_field_->clear();
        input_field_->setFixedHeight(48);  // comfortable default after send, prevents shrinking too small
    }
    return text;
}

void ChatView::focus_input() {
    input_field_->setFocus();
}

// ── Bubble Creation ────────────────────────────────────────────────

QWidget* ChatView::create_bubble(const QString& username, const QString& content,
                                 qint64 timestamp, bool is_self) {
    auto* container = new QWidget;
    container->setObjectName(is_self ? QStringLiteral("bubbleSelf")
                                     : QStringLiteral("bubbleOther"));

    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(10, 14, 10, 14);  // per design spec
    layout->setSpacing(2);

    // Username row
    auto* name_label = new QLabel(username);
    name_label->setObjectName(QStringLiteral("msgUsername"));
    layout->addWidget(name_label);

    // Content
    auto* content_label = new QLabel(content);
    content_label->setObjectName(QStringLiteral("msgContent"));
    content_label->setWordWrap(true);
    content_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(content_label);

    // Timestamp
    QDateTime dt = QDateTime::fromSecsSinceEpoch(timestamp);
    auto* ts_label = new QLabel(dt.toString(QStringLiteral("HH:mm:ss")));
    ts_label->setObjectName(QStringLiteral("msgTimestamp"));
    layout->addWidget(ts_label);

    // Avatar per design (28px circle with initial)
    auto* avatar = new QLabel;
    avatar->setFixedSize(28, 28);
    avatar->setAlignment(Qt::AlignCenter);
    QString initial = username.isEmpty() ? "?" : username.left(1).toUpper();
    avatar->setText(initial);
    avatar->setObjectName(is_self ? QStringLiteral("avatarSelf") : QStringLiteral("avatarOther"));

    // Wrap avatar + container
    auto* wrapper = new QWidget;
    auto* wrapper_layout = new QHBoxLayout(wrapper);
    wrapper_layout->setContentsMargins(0, 0, 0, 0);
    wrapper_layout->setSpacing(8);

    if (is_self) {
        wrapper_layout->addStretch();
        wrapper_layout->addWidget(container);
        container->setMaximumWidth(360);
        wrapper_layout->addWidget(avatar);
    } else {
        wrapper_layout->addWidget(avatar);
        wrapper_layout->addWidget(container);
        container->setMaximumWidth(360);
        wrapper_layout->addStretch();
    }

    return wrapper;
}

QWidget* ChatView::create_system_bubble(const QString& content) {
    auto* wrapper = new QWidget;
    auto* layout = new QHBoxLayout(wrapper);
    layout->setContentsMargins(0, 4, 0, 4);

    layout->addStretch();

    auto* label = new QLabel(content);
    label->setObjectName(QStringLiteral("systemNotify"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    layout->addStretch();

    return wrapper;
}

QWidget* ChatView::create_time_separator(const QString& time_text) {
    auto* wrapper = new QWidget;
    auto* layout = new QHBoxLayout(wrapper);
    layout->setContentsMargins(0, 8, 0, 8);

    auto* line_left = new QFrame;
    line_left->setFrameShape(QFrame::HLine);
    line_left->setObjectName(QStringLiteral("timeSeparatorLine"));
    layout->addWidget(line_left, 1);

    auto* label = new QLabel(time_text);
    label->setObjectName(QStringLiteral("timeSeparator"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    auto* line_right = new QFrame;
    line_right->setFrameShape(QFrame::HLine);
    line_right->setObjectName(QStringLiteral("timeSeparatorLine"));
    layout->addWidget(line_right, 1);

    return wrapper;
}

// ── Test / Direct Chat Support (for --test-chat QSS iteration) ─────

void ChatView::populateTestData() {
    // Rich reproducible mock data. Call once after show(). Does not clear prior.
    const qint64 now = QDateTime::currentSecsSinceEpoch();

    // Mix of short, long, multi-line, emoji-ish, system
    append_message(QStringLiteral("测试用户"), QStringLiteral("你好，这是测试模式下的自发消息。"), now - 300, true);
    append_message(QStringLiteral("Alice"), QStringLiteral("收到了！界面看起来很清爽。"), now - 280, false);
    append_system_message(QStringLiteral("测试用户 加入了聊天 • 3 participants"));

    append_message(QStringLiteral("Bob"), QStringLiteral("这个 QSS 改动后感觉专业多了，间距和圆角都更现代。支持一下。"), now - 200, false);
    append_message(QStringLiteral("测试用户"), QStringLiteral("是的，目标是干净的深色专业风格 + 良好对比 + 8px 圆角。直接用 --test-chat 快速迭代。"), now - 170, true);

    const QString longMsg = QStringLiteral("这是一段较长的消息，用于测试气泡换行、内容宽度限制和滚动行为。"
        "在新的设计系统中，气泡使用 elevated surface + 微边框，左/右对齐通过 wrapper 实现。"
        "发送和接收使用不同的背景色和文字色。");
    append_message(QStringLiteral("测试用户"), longMsg, now - 120, true);

    append_message(QStringLiteral("Alice"), QStringLiteral("👍 收到，多试试 hover / focus 状态。"), now - 90, false);
    append_system_message(QStringLiteral("Bob 离开了"));

    append_message(QStringLiteral("测试用户"), QStringLiteral("输入框和发送按钮的交互也要验证。"), now - 40, true);
    append_message(QStringLiteral("Carol"), QStringLiteral("完美。改 QSS 后 rebuild + 直接 -t 就能看到效果。"), now - 10, false);
}
