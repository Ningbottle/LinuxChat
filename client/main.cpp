// main.cpp - LinuxChat Qt6 client entry point
//
// Flow:
//   1. Initialize QApplication + high-DPI support
//   2. Load custom fonts via FontManager
//   3. Load global stylesheet from Qt resources (:/style.qss)
//   4. Show LoginDialog -> wait for authentication
//   5. On success, show MainWindow

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QStyleFactory>

#include "chat_client.h"
#include "font_manager.h"
#include "login_dialog.h"
#include "main_window.h"

int main(int argc, char* argv[]) {
    // High-DPI rounding policy (Qt6)
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("LinuxChat"));
    app.setApplicationVersion(QStringLiteral("1.0"));

    // Load custom fonts
    if (!FontManager::instance().loadFonts()) {
        qWarning() << "Failed to load custom fonts, using system defaults";
    }

    // Load global QSS stylesheet from Qt resources
    QFile style_file(QStringLiteral(":/style.qss"));
    if (style_file.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(style_file.readAll());
        style_file.close();
        qDebug() << "Style sheet loaded successfully";
    } else {
        qWarning() << "Failed to load style.qss:" << style_file.errorString();
    }

    // Create shared protocol client
    ChatClient client;

    // Show login dialog
    LoginDialog login(&client);
    if (login.exec() != QDialog::Accepted) {
        return 0; // User cancelled
    }

    // Login successful - show main window
    MainWindow window(&client, login.username());
    window.show();

    // Request initial history load
    client.send_history_req(QStringLiteral("__room__"));

    return app.exec();
}
