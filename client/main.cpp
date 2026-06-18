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
    // Prevent Qt from quitting when LoginDialog closes (MainWindow not shown yet)
    app.setQuitOnLastWindowClosed(false);

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

    // --- Direct chat UI bypass for fast QSS testing (no server needed) ---
    // Parse simple CLI flags early: --test-chat | --direct-chat | -t
    bool isTestMode = false;
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]).toLower();
        if (arg == "--test-chat" || arg == "--direct-chat" || arg == "-t") {
            isTestMode = true;
            break;
        }
    }

    if (isTestMode) {
        qDebug() << "[main] --test-chat mode: launching direct chat UI (no login, no server)";
        ChatClient client;  // real instance but no connect; sends are no-op
        MainWindow window(&client, QStringLiteral("DemoUser"), /*testMode=*/true);
        window.show();
        window.populateTestData();

        // Live echo wiring for interactive Send testing (input + bubble styles)
        // Connect room view send to local append (also privates get echo via their wiring)
        // We access via a small post-show hook (room is first tab)
        // For simplicity the populate + extra lambda below gives immediate feedback.
        QObject::connect(&window, &MainWindow::returnToLoginRequested, [&app, &client]() {
            qDebug() << "[main] test mode: returnToLoginRequested - returning to login UI state";
            // Simulate back to login (shows login dialog to fulfill "断开后回到登录状态")
            LoginDialog login(&client);
            login.exec();
        });

        // Simple direct echo for the primary room view send (covers input/send/bubble QSS)
        // Note: full tab wiring done inside populateTestData + get_or_create
        qDebug() << "[main] test mode ready. Edit style.qss, rebuild, rerun with -t to iterate.";
        return app.exec();
    }

    // Create shared protocol client (normal path)
    ChatClient client;

    // Loop to support return to login after disconnect
    while (true) {
        // Show login dialog
        LoginDialog login(&client);
        if (login.exec() != QDialog::Accepted) {
            break; // User cancelled
        }

        // Login successful - show main window
        qDebug() << "[main] Login successful, creating MainWindow";
        MainWindow window(&client, login.username());
        qDebug() << "[main] Showing MainWindow";

        bool wantReLogin = false;
        QObject::connect(&window, &MainWindow::returnToLoginRequested, [&wantReLogin, &window]() {
            wantReLogin = true;
            window.close();
        });

        window.show();

        // Request initial history load
        qDebug() << "[main] Sending history request";
        client.send_history_req(QStringLiteral("__room__"));

        qDebug() << "[main] Starting event loop for this session";
        app.exec();

        if (!wantReLogin) {
            break;
        }
        qDebug() << "[main] Returning to login after disconnect";
    }

    qDebug() << "[main] Exiting application";
    return 0;
}
