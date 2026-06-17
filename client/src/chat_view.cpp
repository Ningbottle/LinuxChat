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
    message_layout_->setContentsMargins(16, 12, 16, 12);
    message_layout_->setSpacing(8);
    message_layout_->addStretch(); // Push messages to top

    scroll_area_->setWidget(scroll_widget_);
    outer->addWidget(scroll_area_, 1);

    // Input area
    auto* input_layout = new QHBoxLayout;
    input_layout->setContentsMargins(12, 8, 12, 12);
    input_layout->setSpacing(8);

    input_field_ = new QTextEdit;
    input_field_->setObjectName(QStringLiteral("inputField"));
    input_field_->setPlaceholderText(QStringLiteral("输入消息..."));
    input_field_->setMaximumHeight(80);
    input_field_->setTabChangesFocus(true);
    input_layout->addWidget(input_field_, 1);

    send_btn_ = new QPushButton(QStringLiteral("发送"));
    send_btn_->setObjectName(QStringLiteral("primaryBtn"));
    send_btn_->setFixedWidth(80);
    send_btn_->setFixedHeight(36);
    input_layout->addWidget(send_btn_);

    outer->addLayout(input_layout);

    // Connect send button
    connect(send_btn_, &QPushButton::clicked, this, &ChatView::send_requested);

    // Ctrl+Enter to send
    connect(input_field_, &QTextEdit::textChanged, this, [this]() {
        // Auto-resize input field (max 80px)
        int doc_height = input_field_->document()->size().height();
        input_field_->setFixedHeight(qBound(36, doc_height + 12, 80));
    });
}

// ── Public API ─────────────────────────────────────────────────────

void ChatView::append_message(const QString& username, const QString& content,
                              qint64 timestamp, bool is_self) {
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
        input_field_->setFixedHeight(36);
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
    layout->setContentsMargins(12, 8, 12, 8);
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

    // Wrap in alignment container
    auto* wrapper = new QWidget;
    auto* wrapper_layout = new QHBoxLayout(wrapper);
    wrapper_layout->setContentsMargins(0, 0, 0, 0);

    if (is_self) {
        wrapper_layout->addStretch();
        wrapper_layout->addWidget(container);
        container->setMaximumWidth(360);
    } else {
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
