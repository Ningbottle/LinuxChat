#pragma once
// chat_backend.h — QML-accessible facade wrapping ChatClient for LinuxChat
//
// Bridges the existing ChatClient protocol layer to QML via Q_PROPERTY
// and Q_INVOKABLE methods. Owns and exposes data models (MessageModel,
// UserModel, SessionModel) so QML views can bind to them directly.
// Designed for incremental migration from Qt Widgets to QML/Qt Quick.

#include <QObject>
#include <QString>
#include <QMap>

#include "chat_client.h"
#include "message_model.h"
#include "user_model.h"
#include "session_model.h"

class ChatBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(QString currentUser READ currentUser NOTIFY currentUserChanged)
    Q_PROPERTY(MessageModel* roomMessages READ roomMessages CONSTANT)
    Q_PROPERTY(UserModel* onlineUsers READ onlineUsers CONSTANT)
    Q_PROPERTY(SessionModel* sessions READ sessions CONSTANT)

public:
    explicit ChatBackend(ChatClient* client, QObject* parent = nullptr);
    ~ChatBackend() override;

    // ── Server Interaction ──────────────────────────────────────────

    Q_INVOKABLE void connectToServer(const QString& host, int port);
    Q_INVOKABLE void login(const QString& username, const QString& password);
    Q_INVOKABLE void registerUser(const QString& username, const QString& password);
    Q_INVOKABLE void sendMessage(const QString& content);
    Q_INVOKABLE void sendPrivate(const QString& to, const QString& content);
    Q_INVOKABLE void requestHistory(const QString& target);
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE void logout();
    Q_INVOKABLE void setCurrentUser(const QString& username);
    Q_INVOKABLE void populateTestData();

    // ── Session Model Proxies (for QML convenience) ──────────────────

    Q_INVOKABLE void addSession(const QString& targetName, bool isRoom);
    Q_INVOKABLE void clearUnread(const QString& targetName);

    // ── Model Accessors ─────────────────────────────────────────────

    [[nodiscard]] MessageModel* roomMessages() const;
    [[nodiscard]] UserModel* onlineUsers() const;
    [[nodiscard]] SessionModel* sessions() const;

    /// Returns the MessageModel for a private conversation with `username`.
    /// Creates one if it does not yet exist. Ownership stays with ChatBackend.
    Q_INVOKABLE MessageModel* privateMessages(const QString& username);

    // ── Property Getters ────────────────────────────────────────────

    [[nodiscard]] QString connectionStatus() const;
    [[nodiscard]] QString currentUser() const;

signals:
    void connectionStatusChanged();
    void currentUserChanged();
    void messageReceived(const QString& sender, const QString& content, const QString& timestamp);
    void loginSuccess();
    void loginFailed(const QString& reason);
    void errorOccurred(const QString& message);
    void userListUpdated(const QStringList& users);

private:
    void setupConnections();

    ChatClient* m_client = nullptr;

    QString m_connectionStatus = QStringLiteral("Disconnected");
    QString m_currentUser;

    SessionModel* m_sessions = nullptr;
    MessageModel* m_roomMessages = nullptr;
    UserModel* m_onlineUsers = nullptr;

    /// Per-user private conversation models, keyed by the other user's name.
    QMap<QString, MessageModel*> m_privateMessages;
};
