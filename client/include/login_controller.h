#pragma once
// login_controller.h — QML-friendly login/register controller
//
// Manages connection and authentication state machines, with timeout
// guards ported from LoginDialog.  Exposes Q_PROPERTYs that QML binds
// to for status text, loading spinners, and error display.

#include <QObject>
#include <QString>
#include <QTimer>

class ChatClient;

class LoginController : public QObject {
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(bool isError READ isError NOTIFY isErrorChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:
    enum State { Disconnected, Connecting, Authenticating, Authenticated };
    Q_ENUM(State)

    explicit LoginController(ChatClient* client, QObject* parent = nullptr);
    ~LoginController() override = default;

    State state() const;
    QString statusText() const;
    bool isError() const;
    bool isLoading() const;

    Q_INVOKABLE void connectToServer(const QString& host, int port);
    Q_INVOKABLE void login(const QString& username, const QString& password);
    Q_INVOKABLE void registerUser(const QString& username, const QString& password);

signals:
    void stateChanged();
    void statusTextChanged();
    void isErrorChanged();
    void isLoadingChanged();
    void authenticated(const QString& username);

private slots:
    void onConnected();
    void onDisconnected();
    void onConnectionError(const QString& error);
    void onLoginOk(const QString& username);
    void onErrorReceived(const QString& code, const QString& content);
    void onConnectTimeout();
    void onLoginTimeout();

private:
    void setState(State state);
    void setStatusText(const QString& text, bool error = false);
    void setLoading(bool loading);

    ChatClient* m_client;
    State m_state = Disconnected;
    QString m_statusText;
    bool m_isError = false;
    bool m_isLoading = false;
    bool m_isConnecting = false;

    QTimer m_connectTimer;  // 10s connection timeout
    QTimer m_loginTimer;    // 8s login/register response timeout
};
