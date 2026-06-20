// chat_backend.cpp — QML facade implementation

#include "chat_backend.h"
#include "chat_client.h"
#include "message_model.h"
#include "user_model.h"
#include "session_model.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

ChatBackend::ChatBackend(ChatClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_sessions(new SessionModel(this))
    , m_roomMessages(new MessageModel(this))
    , m_onlineUsers(new UserModel(this))
{
    setupConnections();
}

ChatBackend::~ChatBackend()
{
    // m_privateMessages models are children of this and auto-deleted by Qt.
}

// ── Signal Wiring ──────────────────────────────────────────────────

void ChatBackend::setupConnections()
{
    // Connection state changes
    connect(m_client, &ChatClient::connected, this, [this]() {
        m_connectionStatus = QStringLiteral("Connected");
        emit connectionStatusChanged();
    });

    connect(m_client, &ChatClient::disconnected, this, [this]() {
        m_connectionStatus = QStringLiteral("Disconnected");
        emit connectionStatusChanged();
    });

    connect(m_client, &ChatClient::connection_error, this, [this](const QString& err) {
        m_connectionStatus = QStringLiteral("Error: ") + err;
        emit connectionStatusChanged();
        emit errorOccurred(err);
    });

    // Auth results
    connect(m_client, &ChatClient::login_ok, this, [this](const QString& username) {
        m_currentUser = username;
        emit currentUserChanged();
        emit loginSuccess();
    });

    connect(m_client, &ChatClient::error_received, this,
            [this](const QString& code, const QString& content) {
                Q_UNUSED(code)
                emit loginFailed(content);
                emit errorOccurred(content);
            });

    // Broadcast (room) messages → m_roomMessages
    connect(m_client, &ChatClient::broadcast_received, this,
            [this](const QString& from, const QString& content, qint64 timestamp) {
                const auto ts = QDateTime::fromSecsSinceEpoch(timestamp)
                                    .toString(QStringLiteral("hh:mm:ss"));
                m_roomMessages->addMessage(from, content, ts, from == m_currentUser);
                emit messageReceived(from, content, ts);
            });

    // Private messages → per-user model
    connect(m_client, &ChatClient::private_received, this,
            [this](const QString& from, const QString& to,
                   const QString& content, qint64 timestamp) {
                Q_UNUSED(to)
                const auto ts = QDateTime::fromSecsSinceEpoch(timestamp)
                                    .toString(QStringLiteral("hh:mm:ss"));
                MessageModel* model = privateMessages(from);
                model->addMessage(from, content, ts, false);
                emit messageReceived(from, content, ts);
            });

    // Online user list → m_onlineUsers
    connect(m_client, &ChatClient::user_list_updated, this,
            [this](const QStringList& users) {
                m_onlineUsers->setUsers(users);
                emit userListUpdated(users);
            });

    // History → load into the appropriate model
    connect(m_client, &ChatClient::history_received, this,
            [this](const QString& target, const QJsonArray& messages) {
                MessageModel* model = nullptr;
                if (target == QStringLiteral("__room__")) {
                    model = m_roomMessages;
                } else {
                    model = privateMessages(target);
                }
                // loadFromJsonArray will be added to MessageModel by
                // the companion agent; it replaces the model contents
                // with the deserialized history entries.
                model->loadFromJsonArray(messages, m_currentUser);
            });

    // Server notifications → system message in room
    connect(m_client, &ChatClient::notify_received, this,
            [this](const QString& content) {
                const auto ts = QDateTime::currentDateTime()
                                    .toString(QStringLiteral("hh:mm:ss"));
                m_roomMessages->addMessage(
                    QStringLiteral("__system__"), content, ts, false,
                    QStringLiteral("system"));
            });
}

// ── QML-Invokable Server Actions ───────────────────────────────────

void ChatBackend::connectToServer(const QString& host, int port)
{
    m_connectionStatus = QStringLiteral("Connecting...");
    emit connectionStatusChanged();
    m_client->connect_to_server(host, static_cast<quint16>(port));
}

void ChatBackend::login(const QString& username, const QString& password)
{
    m_client->send_login(username, password);
}

void ChatBackend::registerUser(const QString& username, const QString& password)
{
    m_client->send_register(username, password);
}

void ChatBackend::sendMessage(const QString& content)
{
    m_client->send_broadcast(content);
}

void ChatBackend::sendPrivate(const QString& to, const QString& content)
{
    m_client->send_private(to, content);

    // Echo the sent message into the local private model so QML sees it
    // immediately without waiting for a server round-trip.
    const auto ts = QDateTime::currentDateTime()
                        .toString(QStringLiteral("hh:mm:ss"));
    MessageModel* model = privateMessages(to);
    model->addMessage(m_currentUser, content, ts, true);
}

void ChatBackend::requestHistory(const QString& target)
{
    m_client->send_history_req(target);
}

void ChatBackend::disconnectFromServer()
{
    m_client->disconnect_from_server();
}

void ChatBackend::logout()
{
    m_client->send_logout();
    m_currentUser.clear();
    emit currentUserChanged();
}

void ChatBackend::setCurrentUser(const QString& username)
{
    if (m_currentUser != username) {
        m_currentUser = username;
        emit currentUserChanged();
    }
}

void ChatBackend::populateTestData()
{
    const QString self = m_currentUser.isEmpty() ? QStringLiteral("我") : m_currentUser;
    m_roomMessages->addMessage(QStringLiteral("陈晓雨"), QStringLiteral("大家好！有人试过新的文件分享功能吗？"), QStringLiteral("14:30"), false);
    m_roomMessages->addMessage(QStringLiteral("张伟"), QStringLiteral("试过了！效果很好，上传速度很给力。"), QStringLiteral("14:31"), false);
    m_roomMessages->addMessage(self, QStringLiteral("我也刚测试了，拖拽上传非常流畅，大家干得漂亮！"), QStringLiteral("14:32"), true);
    m_roomMessages->addMessage(QStringLiteral("李梦琪"), QStringLiteral("不过文档得更新一下，我发现有几张截图已经过时了。"), QStringLiteral("14:33"), false);
    m_roomMessages->addMessage(QStringLiteral("__system__"), QStringLiteral("王浩然 加入了聊天"), QStringLiteral("14:34"), false);
    m_roomMessages->addMessage(QStringLiteral("王浩然"), QStringLiteral("大家好！刚看到更新通知，新的毛玻璃界面太好看了。"), QStringLiteral("14:34"), false);
    m_roomMessages->addMessage(QStringLiteral("刘思远"), QStringLiteral("哇，两个配色方案都很好看！我更喜欢蓝色渐变那个。"), QStringLiteral("14:36"), false);
    m_onlineUsers->setUsers({QStringLiteral("陈晓雨"), QStringLiteral("张伟"), QStringLiteral("李梦琪"), QStringLiteral("王浩然"), QStringLiteral("刘思远")});
    m_sessions->addSession(QStringLiteral("陈晓雨"), false);
    m_sessions->addSession(QStringLiteral("张伟"), false);
}

// ── Model Accessors ────────────────────────────────────────────────

MessageModel* ChatBackend::roomMessages() const
{
    return m_roomMessages;
}

UserModel* ChatBackend::onlineUsers() const
{
    return m_onlineUsers;
}

SessionModel* ChatBackend::sessions() const
{
    return m_sessions;
}

MessageModel* ChatBackend::privateMessages(const QString& username)
{
    auto it = m_privateMessages.find(username);
    if (it != m_privateMessages.end()) {
        return it.value();
    }
    auto* model = new MessageModel(this);
    m_privateMessages.insert(username, model);
    return model;
}

// ── Property Getters ───────────────────────────────────────────────

QString ChatBackend::connectionStatus() const
{
    return m_connectionStatus;
}

QString ChatBackend::currentUser() const
{
    return m_currentUser;
}

// ── Session Model Proxies ──────────────────────────────────────────

void ChatBackend::addSession(const QString& targetName, bool isRoom)
{
    m_sessions->addSession(targetName, isRoom);
}

void ChatBackend::clearUnread(const QString& targetName)
{
    m_sessions->clearUnread(targetName);
}
