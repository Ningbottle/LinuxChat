// login_controller.cpp — Implementation of LoginController
//
// Timeout and error handling logic is ported from LoginDialog so that
// the QML login page behaves identically to the original widget-based UI.

#include "login_controller.h"
#include "chat_client.h"

LoginController::LoginController(ChatClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
{
    // --- Timers (single-shot, matching original dialog timeouts) ---
    m_connectTimer.setSingleShot(true);
    m_connectTimer.setInterval(10000);  // 10 seconds
    m_loginTimer.setSingleShot(true);
    m_loginTimer.setInterval(8000);     // 8 seconds

    connect(&m_connectTimer, &QTimer::timeout,
            this, &LoginController::onConnectTimeout);
    connect(&m_loginTimer, &QTimer::timeout,
            this, &LoginController::onLoginTimeout);

    // --- ChatClient signals ---
    connect(m_client, &ChatClient::connected,
            this, &LoginController::onConnected);
    connect(m_client, &ChatClient::disconnected,
            this, &LoginController::onDisconnected);
    connect(m_client, &ChatClient::connection_error,
            this, &LoginController::onConnectionError);
    connect(m_client, &ChatClient::login_ok,
            this, &LoginController::onLoginOk);
    connect(m_client, &ChatClient::error_received,
            this, &LoginController::onErrorReceived);
}

// ── Property accessors ──────────────────────────────────────────────

LoginController::State LoginController::state() const { return m_state; }
QString LoginController::statusText() const { return m_statusText; }
bool LoginController::isError() const { return m_isError; }
bool LoginController::isLoading() const { return m_isLoading; }

// ── Q_INVOKABLE actions ─────────────────────────────────────────────

void LoginController::connectToServer(const QString& host, int port)
{
    // Guard against re-entrant calls (rapid clicks or signal re-entry).
    if (m_isConnecting) {
        return;
    }

    if (host.isEmpty() || port <= 0 || port > 65535) {
        setStatusText(QStringLiteral("请输入有效的服务器地址和端口"), true);
        return;
    }

    m_isConnecting = true;
    setStatusText(QStringLiteral("正在连接..."), false);
    setLoading(true);
    setState(Connecting);
    m_connectTimer.start();
    m_client->connect_to_server(host, static_cast<quint16>(port));
}

void LoginController::login(const QString& username, const QString& password)
{
    if (username.trimmed().isEmpty() || password.trimmed().isEmpty()) {
        setStatusText(QStringLiteral("请输入用户名和密码"), true);
        return;
    }
    if (!m_client->is_connected()) {
        setStatusText(QStringLiteral("请先连接到服务器"), true);
        return;
    }

    setStatusText(QStringLiteral(""), false);
    setLoading(true);
    setState(Authenticating);
    m_loginTimer.start();
    m_client->send_login(username.trimmed(), password.trimmed());
}

void LoginController::registerUser(const QString& username, const QString& password)
{
    if (username.trimmed().isEmpty() || password.trimmed().isEmpty()) {
        setStatusText(QStringLiteral("请输入用户名和密码"), true);
        return;
    }
    if (!m_client->is_connected()) {
        setStatusText(QStringLiteral("请先连接到服务器"), true);
        return;
    }

    setStatusText(QStringLiteral(""), false);
    setLoading(true);
    setState(Authenticating);
    m_loginTimer.start();
    m_client->send_register(username.trimmed(), password.trimmed());
}

// ── ChatClient signal handlers ──────────────────────────────────────

void LoginController::onConnected()
{
    m_isConnecting = false;
    m_connectTimer.stop();
    setLoading(false);
    setStatusText(QStringLiteral("已连接到服务器"), false);
    setState(Authenticating);
}

void LoginController::onDisconnected()
{
    m_isConnecting = false;
    m_connectTimer.stop();
    m_loginTimer.stop();
    setLoading(false);
    setStatusText(QStringLiteral("与服务器断开连接"), true);
    setState(Disconnected);
}

void LoginController::onConnectionError(const QString& error)
{
    m_isConnecting = false;
    m_connectTimer.stop();
    m_loginTimer.stop();
    setLoading(false);
    setStatusText(QStringLiteral("连接失败: %1").arg(error), true);
    setState(Disconnected);
}

void LoginController::onLoginOk(const QString& username)
{
    m_connectTimer.stop();
    m_loginTimer.stop();
    setLoading(false);
    setStatusText(QStringLiteral(""), false);
    setState(Authenticated);
    emit authenticated(username);
}

void LoginController::onErrorReceived(const QString& code, const QString& content)
{
    m_loginTimer.stop();
    setLoading(false);
    setStatusText(QStringLiteral("[%1] %2").arg(code, content), true);
    // Reset to Authenticating so user can retry login/register without reconnecting
    setState(Authenticating);
}

// ── Timer timeout handlers ──────────────────────────────────────────

void LoginController::onConnectTimeout()
{
    m_isConnecting = false;
    setLoading(false);
    setStatusText(QStringLiteral("连接超时"), true);
    setState(Disconnected);
}

void LoginController::onLoginTimeout()
{
    setLoading(false);
    setStatusText(QStringLiteral("登录超时"), true);
    // Stay in Authenticating state so user can retry without reconnecting.
}

// ── Private helpers ─────────────────────────────────────────────────

void LoginController::setState(State state)
{
    if (m_state == state)
        return;
    m_state = state;
    emit stateChanged();
}

void LoginController::setStatusText(const QString& text, bool error)
{
    if (m_statusText == text && m_isError == error)
        return;
    m_statusText = text;
    m_isError = error;
    emit statusTextChanged();
    emit isErrorChanged();
}

void LoginController::setLoading(bool loading)
{
    if (m_isLoading == loading)
        return;
    m_isLoading = loading;
    emit isLoadingChanged();
}
