// chat_client.cpp — ChatClient implementation
//
// Frame format (mirrors server protocol.cpp):
//   [4-byte big-endian length][JSON UTF-8 body]

#include "chat_client.h"
#include <QtEndian>  // qToBigEndian, qFromBigEndian
#include <QTimer>
#include <QDebug>
#include <cstring>

// Windows SO_LINGER support for forcing RST instead of FIN
#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

// ── Constructor ────────────────────────────────────────────────────

ChatClient::ChatClient(QObject* parent)
    : QObject(parent)
{
    // Socket is created lazily in connect_to_server() to avoid Windows
    // socket descriptor reuse race condition (see analysis report).
    socket_ = nullptr;
}

// ── Private: Socket Setup ─────────────────────────────────────────

void ChatClient::setup_socket() {
    // Destroy old socket if exists
    if (socket_) {
        socket_->blockSignals(true);

        // CRITICAL: Force RST instead of FIN before abort().
        // On Windows, Qt's abort() calls closesocket() which sends FIN asynchronously.
        // If the new socket reuses the same local port, the delayed FIN from the old
        // socket corrupts the new connection (server sees recv n=0).
        // Setting SO_LINGER {1,0} forces closesocket() to send RST immediately.
#ifdef Q_OS_WIN
        if (socket_->state() != QAbstractSocket::UnconnectedState) {
            qintptr fd = socket_->socketDescriptor();
            if (fd != -1) {
                struct linger lg;
                lg.l_onoff  = 1;
                lg.l_linger = 0;
                ::setsockopt(static_cast<SOCKET>(fd), SOL_SOCKET, SO_LINGER,
                             reinterpret_cast<const char*>(&lg), sizeof(lg));
            }
        }
#endif
        socket_->abort();

        // CRITICAL: Use synchronous delete, NOT deleteLater().
        // deleteLater() defers destruction to the next event loop iteration.
        // When connectToHost() runs, the event loop processes both the deferred
        // delete of the old socket AND the new socket's connection events in
        // undefined order. The old socket's destructor can corrupt Qt's internal
        // socket engine state, killing the new connection.
        // With abort() + SO_LINGER, the native fd is already closed and RST sent,
        // so synchronous delete only cleans up the Qt object wrapper.
        delete socket_;
        socket_ = nullptr;
    }

    // Create fresh socket for each connection attempt
    socket_ = new QTcpSocket(this);

    connect(socket_, &QTcpSocket::connected,
            this, &ChatClient::on_socket_connected);
    connect(socket_, &QTcpSocket::disconnected,
            this, &ChatClient::on_socket_disconnected);
    connect(socket_, &QTcpSocket::readyRead,
            this, &ChatClient::on_ready_read);
    connect(socket_, &QAbstractSocket::errorOccurred,
            this, &ChatClient::on_socket_error);
}

// ── Connection Management ──────────────────────────────────────────

void ChatClient::connect_to_server(const QString& host, quint16 port) {
    // Guard against re-entrant connection attempts
    if (is_connecting_) {
        qDebug("[ChatClient] connect_to_server: already connecting, ignoring re-entrant call");
        return;
    }
    is_connecting_ = true;

    qDebug("[ChatClient] Connecting to %s:%d", qUtf8Printable(host), port);
    recv_buf_.clear();

    // Create fresh socket to avoid Windows fd reuse race condition
    setup_socket();

    // connectToHost() is async - connection completes when connected() signal fires
    socket_->connectToHost(host, port);

    // If connectToHost() failed synchronously (e.g. bad host), Qt moves the
    // socket to UnconnectedState without emitting connected().  In that case
    // the connection_error signal must still be emitted so the UI can recover.
    if (socket_->state() == QAbstractSocket::UnconnectedState) {
        is_connecting_ = false;
        emit connection_error(socket_->errorString());
    }
}

void ChatClient::disconnect_from_server() {
    // CRITICAL: Use abort() + SO_LINGER, NOT disconnectFromHost().
    // disconnectFromHost() sends a graceful FIN that stays in-flight on the network.
    // If the user reconnects immediately, Windows may reuse the same local ephemeral
    // port for the new connection. The stale FIN from the old connection then arrives
    // at the server and matches the new connection's 4-tuple, causing recv() to
    // return 0 immediately (the "recv n=0" bug).
    // abort() + SO_LINGER {1,0} sends RST instead of FIN, and releases the port
    // immediately with no in-flight segments.
    if (socket_) {
#ifdef Q_OS_WIN
        if (socket_->state() != QAbstractSocket::UnconnectedState) {
            qintptr fd = socket_->socketDescriptor();
            if (fd != -1) {
                struct linger lg;
                lg.l_onoff  = 1;
                lg.l_linger = 0;
                ::setsockopt(static_cast<SOCKET>(fd), SOL_SOCKET, SO_LINGER,
                             reinterpret_cast<const char*>(&lg), sizeof(lg));
            }
        }
#endif
        socket_->abort();
    }
}

bool ChatClient::is_connected() const {
    return socket_ && socket_->state() == QAbstractSocket::ConnectedState;
}

QAbstractSocket::SocketState ChatClient::socketState() const {
    return socket_ ? socket_->state() : QAbstractSocket::UnconnectedState;
}

// ── Send Protocol Messages ─────────────────────────────────────────

void ChatClient::send_register(const QString& username, const QString& password) {
    QJsonObject msg;
    msg["type"]    = QStringLiteral("REGISTER");
    msg["from"]    = username;
    msg["content"] = password;
    send_json(msg);
}

void ChatClient::send_login(const QString& username, const QString& password) {
    QJsonObject msg;
    msg["type"]    = QStringLiteral("LOGIN");
    msg["from"]    = username;
    msg["content"] = password;
    send_json(msg);
}

void ChatClient::send_logout() {
    QJsonObject msg;
    msg["type"] = QStringLiteral("LOGOUT");
    send_json(msg);
}

void ChatClient::send_broadcast(const QString& content) {
    QJsonObject msg;
    msg["type"]    = QStringLiteral("BROADCAST");
    msg["content"] = content;
    send_json(msg);
}

void ChatClient::send_private(const QString& to, const QString& content) {
    QJsonObject msg;
    msg["type"]    = QStringLiteral("PRIVATE");
    msg["to"]      = to;
    msg["content"] = content;
    send_json(msg);
}

void ChatClient::send_history_req(const QString& target) {
    QJsonObject msg;
    msg["type"] = QStringLiteral("HISTORY_REQ");
    msg["to"]   = target;
    send_json(msg);
}

// ── Private: Frame I/O ─────────────────────────────────────────────

bool ChatClient::send_json(const QJsonObject& msg) {
    if (!is_connected()) return false;

    QJsonDocument doc(msg);
    QByteArray body = doc.toJson(QJsonDocument::Compact);

    // 4-byte big-endian length prefix (manual to avoid any Qt endian issues)
    quint32 body_len = static_cast<quint32>(body.size());

    QByteArray frame;
    frame.reserve(4 + body.size());
    frame.append(static_cast<char>((body_len >> 24) & 0xFF));
    frame.append(static_cast<char>((body_len >> 16) & 0xFF));
    frame.append(static_cast<char>((body_len >>  8) & 0xFF));
    frame.append(static_cast<char>(body_len & 0xFF));
    frame.append(body);

    qint64 written = socket_->write(frame);
    socket_->flush();   // ensure the frame is sent immediately
    return written == frame.size();
}

// ── Private Slots ──────────────────────────────────────────────────

void ChatClient::on_socket_connected() {
    is_connecting_ = false;
    emit connected();
}

void ChatClient::on_socket_disconnected() {
    qDebug("[ChatClient] Socket disconnected, state=%d, error='%s', buf=%d bytes",
           socket_->state(), qUtf8Printable(socket_->errorString()), recv_buf_.size());
    recv_buf_.clear();

    // If we're still in the connecting phase, this disconnect is a side-effect
    // of a failed connection attempt (or residual signal from abort()).  Emit
    // connection_error instead of disconnected() so the UI doesn't attempt a
    // reconnect loop on what is really a connection failure.
    if (is_connecting_) {
        is_connecting_ = false;
        emit connection_error(socket_->errorString());
        return;
    }

    emit disconnected();
}

void ChatClient::on_socket_error(QAbstractSocket::SocketError error) {
    qDebug("[ChatClient] Socket error code=%d: %s", error, qUtf8Printable(socket_->errorString()));

    // If error occurred during connection attempt, handle it directly.
    // For sockets that never connected, Qt may NOT emit disconnected(),
    // which would leave is_connecting_ stuck at true forever.
    if (is_connecting_) {
        is_connecting_ = false;
        emit connection_error(socket_->errorString());
    }
}

void ChatClient::on_ready_read() {
    // Append all available data to receive buffer
    recv_buf_.append(socket_->readAll());
    process_frames();
}

void ChatClient::process_frames() {
    // Extract all complete frames from the buffer
    while (recv_buf_.size() >= 4) {
        // Read 4-byte big-endian length
        quint32 net_len;
        std::memcpy(&net_len, recv_buf_.constData(), 4);
        quint32 body_len = qFromBigEndian(net_len);

        // Guard against zero-length body (protocol violation)
        if (body_len == 0) {
            qWarning("[ChatClient] Zero-length body frame, disconnecting.");
            recv_buf_.clear();
            disconnect_from_server();
            return;
        }

        // Guard against oversized messages (256KB limit, matches server protocol.cpp)
        if (body_len > 256 * 1024) {
            qWarning("[ChatClient] Oversized frame (%u bytes), disconnecting.", body_len);
            recv_buf_.clear();
            disconnect_from_server();  // Use abort()+SO_LINGER, not disconnectFromHost()
            return;
        }

        // Check if we have the full body (use qsizetype to avoid 32-bit truncation)
        if (recv_buf_.size() < static_cast<qsizetype>(4 + body_len)) {
            break; // Incomplete frame — wait for more data
        }

        // Extract the JSON body
        QByteArray json_data = recv_buf_.mid(4, static_cast<int>(body_len));
        recv_buf_.remove(0, 4 + static_cast<int>(body_len));

        // Parse JSON
        QJsonParseError parse_error;
        QJsonDocument doc = QJsonDocument::fromJson(json_data, &parse_error);
        if (parse_error.error != QJsonParseError::NoError) {
            qWarning("[ChatClient] JSON parse error: %s",
                     qUtf8Printable(parse_error.errorString()));
            continue; // Skip malformed message
        }

        if (!doc.isObject()) {
            qWarning("[ChatClient] Received non-object JSON, skipping.");
            continue;
        }

        dispatch_message(doc.object());
    }
}

void ChatClient::dispatch_message(const QJsonObject& msg) {
    QString type = msg["type"].toString();

    if (type == "PING") {
        // Heartbeat: auto-respond with PONG to keep connection alive
        QJsonObject pong;
        pong["type"] = QStringLiteral("PONG");
        send_json(pong);

    } else if (type == "LOGIN_OK") {
        emit login_ok(msg["from"].toString());

    } else if (type == "ERROR") {
        emit error_received(
            msg["code"].toString(),
            msg["content"].toString()
        );

    } else if (type == "BROADCAST") {
        emit broadcast_received(
            msg["from"].toString(),
            msg["content"].toString(),
            static_cast<qint64>(msg["timestamp"].toDouble())
        );

    } else if (type == "PRIVATE") {
        emit private_received(
            msg["from"].toString(),
            msg["to"].toString(),
            msg["content"].toString(),
            static_cast<qint64>(msg["timestamp"].toDouble())
        );

    } else if (type == "USER_LIST") {
        QStringList users;
        const QJsonArray arr = msg["data"].toArray();
        for (const auto& v : arr) {
            users.append(v.toString());
        }
        emit user_list_updated(users);

    } else if (type == "HISTORY_RESP") {
        emit history_received(
            msg["to"].toString(),
            msg["data"].toArray()
        );

    } else if (type == "NOTIFY") {
        emit notify_received(msg["content"].toString());
    }
}
