#pragma once
// chat_client.h — TCP protocol encapsulation for LinuxChat Qt6 client
//
// Wraps QTcpSocket with JSON-over-TCP framing (4-byte BE prefix + JSON body).
// Mirrors the server's protocol.cpp frame format exactly.
// All communication is signal-driven — UI components connect to these signals.

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>

class ChatClient : public QObject {
    Q_OBJECT

public:
    explicit ChatClient(QObject* parent = nullptr);
    ~ChatClient() override = default;

    /// Connect to the LinuxChat server.
    void connect_to_server(const QString& host, quint16 port);

    /// Disconnect from the server.
    void disconnect_from_server();

    /// Whether the TCP socket is currently connected.
    bool is_connected() const;

    QAbstractSocket::SocketState socketState() const;

    // ── Send Protocol Messages ─────────────────────────────────────

    void send_register(const QString& username, const QString& password);
    void send_login(const QString& username, const QString& password);
    void send_logout();
    void send_broadcast(const QString& content);
    void send_private(const QString& to, const QString& content);
    void send_history_req(const QString& target);  // "__room__" or username

signals:
    // ── Connection Signals ─────────────────────────────────────────
    void connected();
    void disconnected();
    void connection_error(const QString& error_text);

    // ── Server Message Signals ─────────────────────────────────────
    void login_ok(const QString& username);
    void error_received(const QString& code, const QString& content);
    void broadcast_received(const QString& from, const QString& content,
                            qint64 timestamp);
    void private_received(const QString& from, const QString& to,
                          const QString& content, qint64 timestamp);
    void user_list_updated(const QStringList& users);
    void history_received(const QString& target, const QJsonArray& messages);
    void notify_received(const QString& content);

private slots:
    void on_socket_connected();
    void on_socket_disconnected();
    void on_socket_error(QAbstractSocket::SocketError error);
    void on_ready_read();

private:
    /// Create fresh socket for each connection (avoids Windows fd reuse race).
    void setup_socket();

    /// Send a JSON message with 4-byte BE length prefix.
    bool send_json(const QJsonObject& msg);

    /// Process complete frames from recv_buf_ and emit signals.
    void process_frames();

    /// Dispatch a single parsed JSON message to the appropriate signal.
    void dispatch_message(const QJsonObject& msg);

    QTcpSocket* socket_ = nullptr;
    QByteArray  recv_buf_;  ///< Accumulates raw TCP data for frame extraction
    bool        is_connecting_ = false;  ///< Guards against re-entrant connects
};
