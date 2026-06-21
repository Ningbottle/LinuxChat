// main.cpp - LinuxChat Qt6 client entry point

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QDebug>

#include "chat_client.h"
#include "chat_backend.h"
#include "font_manager.h"
#include "login_controller.h"
#include "message_model.h"
#include "user_model.h"
#include "session_model.h"
#include "theme_manager.h"

int main(int argc, char* argv[]) {
    // High-DPI rounding policy
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // Set Qt Quick Controls style BEFORE creating QGuiApplication
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    // Disable QML disk cache so file changes are picked up
    qputenv("QML_DISABLE_DISK_CACHE", "1");

    // Parse CLI flags
    bool isTestMode = false;
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]).toLower();
        if (arg == "--test-chat" || arg == "--direct-chat" || arg == "-t") {
            isTestMode = true;
            break;
        }
    }

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("LinuxChat"));
    app.setApplicationVersion(QStringLiteral("1.0"));

    // Load custom fonts
    if (!FontManager::instance().loadFonts()) {
        qWarning() << "Failed to load custom fonts, using system defaults";
    }

    // Create C++ instances
    ChatClient client;
    ChatBackend backend(&client);
    LoginController loginCtrl(&client);
    ThemeManager themeMgr;

    // When LoginController authenticates, tell ChatBackend the current user
    QObject::connect(&loginCtrl, &LoginController::authenticated,
                     &backend, &ChatBackend::setCurrentUser);

    if (isTestMode) {
        QObject::connect(&loginCtrl, &LoginController::authenticated,
                         &backend, &ChatBackend::populateTestData);
    }

    // Set up QML engine
    QQmlApplicationEngine engine;

    // Expose context properties
    engine.rootContext()->setContextProperty(QStringLiteral("chatClient"), &client);
    engine.rootContext()->setContextProperty(QStringLiteral("chatBackend"), &backend);
    engine.rootContext()->setContextProperty(QStringLiteral("loginController"), &loginCtrl);
    engine.rootContext()->setContextProperty(QStringLiteral("themeMgr"), &themeMgr);
    engine.rootContext()->setContextProperty(QStringLiteral("isTestMode"), isTestMode);

    // Add Qt's QML module path and bin path so DLLs resolve
    engine.addImportPath("C:/Qt/6.8.3/mingw_64/qml");
    qputenv("PATH", "C:/Qt/6.8.3/mingw_64/bin;C:/Qt/Tools/mingw1310_64/bin;" + qgetenv("PATH"));

    // Capture QML warnings
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &warnings) {
        for (const auto &w : warnings) {
            fprintf(stderr, "[QML WARNING] %s\n", w.toString().toUtf8().constData());
        }
        fflush(stderr);
    });

    // Load QML from file system
    QString qmlPath = "D:/ChatBox/LinuxChat/client/qml/main.qml";
    fprintf(stderr, "[main.cpp] Loading QML from: %s\n", qmlPath.toUtf8().constData());
    fflush(stderr);

    engine.load(QUrl::fromLocalFile(qmlPath));

    fprintf(stderr, "[main.cpp] Root objects: %d\n", engine.rootObjects().size());
    fflush(stderr);

    if (engine.rootObjects().isEmpty()) {
        fprintf(stderr, "[main.cpp] FAILED to load QML!\n");
        fflush(stderr);
        return -1;
    }

    return app.exec();
}
