# 瓶子交流器 UI 重构实现计划

> **对于代理工作者：** 必需子技能：使用subagent-driven-development（推荐）或executing-plans逐任务实现此计划。步骤使用复选框（`- [ ]`）语法进行跟踪。

**目标：** 将现有UI重构为报纸复古+紫藤萝风格，使用Claude风格字体（霞鹜文楷+Newsreader）

**架构：** 
- 修改QSS样式文件实现视觉重构
- 更新C++代码添加自定义绘制（SVG背景、地球仪）
- 打包字体资源到应用中

**技术栈：** Qt 6, QSS, QPainter, SVG

---

## 文件映射

### 核心样式文件
- **修改：** `resources/style.qss` - 主样式表

### UI组件文件
- **修改：** `src/login_dialog.cpp` - 登录界面逻辑
- **修改：** `include/login_dialog.h` - 登录界面头文件
- **修改：** `src/main_window.cpp` - 主窗口逻辑
- **修改：** `include/main_window.h` - 主窗口头文件
- **修改：** `src/chat_view.cpp` - 聊天视图逻辑
- **修改：** `include/chat_view.h` - 聊天视图头文件

### 资源文件
- **修改：** `resources/resources.qrc` - Qt资源文件
- **创建：** `resources/fonts/` - 字体目录
- **创建：** `resources/images/` - 图片目录（SVG资源）

### 构建配置
- **修改：** `CMakeLists.txt` - 添加字体和图片资源

---

## 任务分解

### 任务1：准备字体资源

**文件：**
- 创建：`resources/fonts/` 目录
- 创建：`resources/fonts/LXGWWenKai-Regular.ttf`
- 创建：`resources/fonts/LXGWWenKai-Medium.ttf`
- 创建：`resources/fonts/Newsreader-VariableFont_opsz,wght.ttf`
- 修改：`resources/resources.qrc`

- [ ] **步骤1：下载字体文件**

从Google Fonts下载字体：
- LXGW WenKai: https://fonts.google.com/specimen/LXGW+WenKai
- Newsreader: https://fonts.google.com/specimen/Newsreader

```bash
# 使用curl或浏览器下载
cd D:\ChatBox\LinuxChat\client\resources
mkdir -p fonts
# 下载LXGW WenKai
curl -L -o fonts/LXGWWenKai-Regular.ttf "https://github.com/lxgw/LxgwWenKai/releases/download/v1.330/LXGWWenKai-Regular.ttf"
curl -L -o fonts/LXGWWenKai-Medium.ttf "https://github.com/lxgw/LxgwWenKai/releases/download/v1.330/LXGWWenKai-Medium.ttf"
# 下载Newsreader
curl -L -o fonts/Newsreader-VariableFont_opsz,wght.ttf "https://fonts.google.com/download?family=Newsreader"
```

- [ ] **步骤2：更新resources.qrc添加字体**

```xml
<RCC>
  <qresource prefix="/">
    <file>style.qss</file>
    <file>fonts/LXGWWenKai-Regular.ttf</file>
    <file>fonts/LXGWWenKai-Medium.ttf</file>
    <file>fonts/Newsreader-VariableFont_opsz,wght.ttf</file>
  </qresource>
</RCC>
```

- [ ] **步骤3：验证资源文件**

运行：`cd D:\ChatBox\LinuxChat\client\build && cmake --build . --config Release`
预期：编译成功，资源被正确包含

- [ ] **步骤4：提交**

```bash
git add resources/fonts/ resources/resources.qrc
git commit -m "feat: add LXGW WenKai and Newsreader font resources"
```

---

### 任务2：创建SVG资源文件

**文件：**
- 创建：`resources/images/globe.svg` - 地球仪
- 创建：`resources/images/wisteria-pattern.svg` - 紫藤萝图案
- 修改：`resources/resources.qrc`

- [ ] **步骤1：创建地球仪SVG**

创建文件 `resources/images/globe.svg`：

```svg
<svg viewBox="0 0 200 200" fill="none" xmlns="http://www.w3.org/2000/svg">
  <circle cx="100" cy="100" r="90" stroke="#78776c" stroke-width="1"/>
  <ellipse cx="100" cy="100" rx="90" ry="35" stroke="#78776c" stroke-width="0.5"/>
  <ellipse cx="100" cy="100" rx="35" ry="90" stroke="#78776c" stroke-width="0.5"/>
  <path d="M10 100 Q100 60 190 100" stroke="#78776c" stroke-width="0.5" fill="none"/>
  <path d="M10 100 Q100 140 190 100" stroke="#78776c" stroke-width="0.5" fill="none"/>
  <line x1="100" y1="10" x2="100" y2="190" stroke="#78776c" stroke-width="0.5"/>
  <line x1="10" y1="100" x2="190" y2="100" stroke="#78776c" stroke-width="0.5"/>
  <ellipse cx="100" cy="100" rx="60" ry="90" stroke="#78776c" stroke-width="0.3"/>
  <ellipse cx="100" cy="100" rx="90" ry="60" stroke="#78776c" stroke-width="0.3"/>
</svg>
```

- [ ] **步骤2：创建紫藤萝图案SVG**

创建文件 `resources/images/wisteria-pattern.svg`：

```svg
<svg width="300" height="300" xmlns="http://www.w3.org/2000/svg">
  <!-- 主藤蔓 - 棕色老藤 -->
  <path d="M0 20 Q80 40 150 30 Q220 20 300 50" stroke="#8b7355" stroke-width="2.5" fill="none"/>
  <path d="M0 80 Q100 60 180 90 Q260 120 300 80" stroke="#8b7355" stroke-width="2" fill="none"/>
  <path d="M50 0 Q30 80 60 150 Q90 220 50 300" stroke="#8b7355" stroke-width="2" fill="none"/>
  <path d="M200 0 Q220 100 190 180 Q160 260 200 300" stroke="#8b7355" stroke-width="1.8" fill="none"/>
  
  <!-- 垂下的花穗 - 紫藤萝瀑布 -->
  <!-- 花穗1 -->
  <path d="M80 30 Q75 60 78 90 Q80 120 76 150" stroke="#c8a2d8" stroke-width="1.5" fill="none"/>
  <circle cx="78" cy="50" r="5" fill="#d8bfd8"/>
  <circle cx="76" cy="65" r="4.5" fill="#e8d5e8"/>
  <circle cx="79" cy="80" r="5" fill="#d8bfd8"/>
  <circle cx="77" cy="95" r="4" fill="#e8d5e8"/>
  <circle cx="78" cy="110" r="4.5" fill="#d8bfd8"/>
  
  <!-- 花穗2 -->
  <path d="M180 80 Q175 110 178 140 Q180 170 176 200" stroke="#c8a2d8" stroke-width="1.5" fill="none"/>
  <circle cx="178" cy="100" r="5" fill="#d8bfd8"/>
  <circle cx="176" cy="115" r="4" fill="#e8d5e8"/>
  <circle cx="179" cy="130" r="5" fill="#d8bfd8"/>
  <circle cx="177" cy="145" r="4.5" fill="#e8d5e8"/>
  <circle cx="178" cy="160" r="4" fill="#d8bfd8"/>
  <circle cx="176" cy="175" r="3.5" fill="#e8d5e8"/>
  
  <!-- 花穗3 -->
  <path d="M260 20 Q255 50 258 80 Q260 110 256 140" stroke="#c8a2d8" stroke-width="1.5" fill="none"/>
  <circle cx="258" cy="40" r="4.5" fill="#d8bfd8"/>
  <circle cx="256" cy="55" r="5" fill="#e8d5e8"/>
  <circle cx="259" cy="70" r="4" fill="#d8bfd8"/>
  <circle cx="257" cy="85" r="5" fill="#e8d5e8"/>
  <circle cx="258" cy="100" r="4.5" fill="#d8bfd8"/>
  
  <!-- 花穗4 - 左侧垂下 -->
  <path d="M60 0 Q55 30 58 60 Q60 90 56 120" stroke="#c8a2d8" stroke-width="1.2" fill="none"/>
  <circle cx="58" cy="20" r="4" fill="#d8bfd8"/>
  <circle cx="56" cy="35" r="4.5" fill="#e8d5e8"/>
  <circle cx="59" cy="50" r="4" fill="#d8bfd8"/>
  <circle cx="57" cy="65" r="3.5" fill="#e8d5e8"/>
  
  <!-- 绿叶 - 茂盛 -->
  <ellipse cx="90" cy="25" rx="12" ry="6" fill="#6b8e6b" transform="rotate(-20 90 25)"/>
  <ellipse cx="70" cy="35" rx="10" ry="5" fill="#7a9b7a" transform="rotate(15 70 35)"/>
  <ellipse cx="190" cy="75" rx="11" ry="5.5" fill="#6b8e6b" transform="rotate(-25 190 75)"/>
  <ellipse cx="165" cy="85" rx="10" ry="5" fill="#7a9b7a" transform="rotate(10 165 85)"/>
  <ellipse cx="270" cy="15" rx="11" ry="5" fill="#6b8e6b" transform="rotate(-15 270 15)"/>
  <ellipse cx="245" cy="30" rx="10" ry="5.5" fill="#7a9b7a" transform="rotate(20 245 30)"/>
  <ellipse cx="55" cy="10" rx="10" ry="5" fill="#6b8e6b" transform="rotate(25 55 10)"/>
  <ellipse cx="40" cy="25" rx="9" ry="4.5" fill="#7a9b7a" transform="rotate(-10 40 25)"/>
  
  <!-- 更多散落的小花 -->
  <circle cx="120" cy="180" r="3" fill="#e8d5e8"/>
  <circle cx="230" cy="200" r="3.5" fill="#d8bfd8"/>
  <circle cx="40" cy="220" r="3" fill="#e8d5e8"/>
  <circle cx="280" cy="160" r="2.5" fill="#d8bfd8"/>
</svg>
```

- [ ] **步骤3：更新resources.qrc添加SVG**

```xml
<RCC>
  <qresource prefix="/">
    <file>style.qss</file>
    <file>fonts/LXGWWenKai-Regular.ttf</file>
    <file>fonts/LXGWWenKai-Medium.ttf</file>
    <file>fonts/Newsreader-VariableFont_opsz,wght.ttf</file>
    <file>images/globe.svg</file>
    <file>images/wisteria-pattern.svg</file>
  </qresource>
</RCC>
```

- [ ] **步骤4：编译验证**

运行：`cd D:\ChatBox\LinuxChat\client\build && cmake --build . --config Release`
预期：编译成功

- [ ] **步骤5：提交**

```bash
git add resources/images/ resources/resources.qrc
git commit -m "feat: add SVG resources for globe and wisteria pattern"
```

---

### 任务3：创建字体加载管理器

**文件：**
- 创建：`include/font_manager.h`
- 创建：`src/font_manager.cpp`
- 修改：`CMakeLists.txt`

- [ ] **步骤1：编写字体管理器头文件**

创建文件 `include/font_manager.h`：

```cpp
#pragma once

#include <QFont>
#include <QString>

class FontManager {
public:
    static FontManager& instance();
    
    // 加载应用字体
    bool loadFonts();
    
    // 获取字体
    QFont bodyFont(int pointSize = 15) const;
    QFont titleFont(int pointSize = 16) const;
    QFont codeFont(int pointSize = 13) const;
    
    // 字体族名称
    QString bodyFontFamily() const { return m_bodyFontFamily; }
    QString titleFontFamily() const { return m_titleFontFamily; }
    QString codeFontFamily() const { return m_codeFontFamily; }

private:
    FontManager();
    ~FontManager() = default;
    
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    QString m_bodyFontFamily;
    QString m_titleFontFamily;
    QString m_codeFontFamily;
};
```

- [ ] **步骤2：编写字体管理器实现**

创建文件 `src/font_manager.cpp`：

```cpp
#include "font_manager.h"
#include <QFontDatabase>
#include <QCoreApplication>
#include <QDebug>

FontManager& FontManager::instance() {
    static FontManager instance;
    return instance;
}

FontManager::FontManager() 
    : m_bodyFontFamily("LXGW WenKai")
    , m_titleFontFamily("Newsreader")
    , m_codeFontFamily("Cascadia Code")
{
}

bool FontManager::loadFonts() {
    // 加载LXGW WenKai Regular
    int id1 = QFontDatabase::addApplicationFont(":/fonts/LXGWWenKai-Regular.ttf");
    if (id1 == -1) {
        qWarning() << "Failed to load LXGWWenKai-Regular.ttf";
    } else {
        QStringList families = QFontDatabase::applicationFontFamilies(id1);
        if (!families.isEmpty()) {
            m_bodyFontFamily = families.first();
            qDebug() << "Loaded font family:" << m_bodyFontFamily;
        }
    }
    
    // 加载LXGW WenKai Medium
    int id2 = QFontDatabase::addApplicationFont(":/fonts/LXGWWenKai-Medium.ttf");
    if (id2 == -1) {
        qWarning() << "Failed to load LXGWWenKai-Medium.ttf";
    }
    
    // 加载Newsreader
    int id3 = QFontDatabase::addApplicationFont(":/fonts/Newsreader-VariableFont_opsz,wght.ttf");
    if (id3 == -1) {
        qWarning() << "Failed to load Newsreader font";
    } else {
        QStringList families = QFontDatabase::applicationFontFamilies(id3);
        if (!families.isEmpty()) {
            m_titleFontFamily = families.first();
            qDebug() << "Loaded title font family:" << m_titleFontFamily;
        }
    }
    
    return true;
}

QFont FontManager::bodyFont(int pointSize) const {
    QFont font(m_bodyFontFamily);
    font.setPointSize(pointSize);
    font.setWeight(QFont::Normal);
    return font;
}

QFont FontManager::titleFont(int pointSize) const {
    QFont font(m_titleFontFamily);
    font.setPointSize(pointSize);
    font.setWeight(QFont::DemiBold);
    return font;
}

QFont FontManager::codeFont(int pointSize) const {
    QFont font(m_codeFontFamily);
    font.setPointSize(pointSize);
    font.setWeight(QFont::Normal);
    return font;
}
```

- [ ] **步骤3：更新CMakeLists.txt**

在 `add_executable` 中添加新文件：

```cmake
# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Header files (MUST be listed for AUTOMOC to find Q_OBJECT)
file(GLOB_RECURSE HEADERS "include/*.h")

# Qt resources
set(RESOURCES resources/resources.qrc)

add_executable(linuxchat_client WIN32
    main.cpp
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)
```

- [ ] **步骤4：编译验证**

运行：`cd D:\ChatBox\LinuxChat\client\build && cmake --build . --config Release`
预期：编译成功

- [ ] **步骤5：提交**

```bash
git add include/font_manager.h src/font_manager.cpp CMakeLists.txt
git commit -m "feat: add FontManager for loading custom fonts"
```

---

### 任务4：更新main.cpp初始化字体

**文件：**
- 修改：`main.cpp`

- [ ] **步骤1：添加字体初始化代码**

修改 `main.cpp`：

```cpp
#include <QApplication>
#include <QFile>
#include <QDebug>
#include "font_manager.h"
#include "login_dialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 加载自定义字体
    if (!FontManager::instance().loadFonts()) {
        qWarning() << "Failed to load custom fonts, using system defaults";
    }
    
    // 加载样式表
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = QString::fromUtf8(styleFile.readAll());
        app.setStyleSheet(style);
        qDebug() << "Style sheet loaded successfully";
    } else {
        qWarning() << "Failed to load style.qss:" << styleFile.errorString();
    }
    
    // 显示登录对话框
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        // 登录成功，进入主窗口
        // MainWindow will be created by LoginDialog
    }
    
    return app.exec();
}
```

- [ ] **步骤2：编译验证**

运行：`cd D:\ChatBox\LinuxChat\client\build && cmake --build . --config Release`
预期：编译成功

- [ ] **步骤3：提交**

```bash
git add main.cpp
git commit -m "feat: initialize FontManager in main.cpp"
```

---

### 任务5：重构style.qss基础样式

**文件：**
- 修改：`resources/style.qss`

- [ ] **步骤1：备份现有样式**

```bash
cd D:\ChatBox\LinuxChat\client\resources
cp style.qss style.qss.backup
```

- [ ] **步骤2：创建新的基础样式**

重写 `resources/style.qss`：

```qss
/* ============================================
   瓶子交流器 - 报纸复古 + 紫藤萝风格
   ============================================ */

/* 全局字体 */
QWidget {
    font-family: "LXGW WenKai", "霞鹜文楷", "PingFang SC", "Microsoft YaHei", serif;
    font-size: 15px;
    color: #1c1917;
}

/* 主窗口 */
QMainWindow {
    background-color: #faf9f7;
}

/* 对话框 */
QDialog {
    background-color: #faf9f7;
}
```

- [ ] **步骤3：编译验证**

运行：`cd D:\ChatBox\LinuxChat\client\build && cmake --build . --config Release`
预期：编译成功

- [ ] **步骤4：提交**

```bash
git add resources/style.qss
git commit -m "refactor: update base style with newspaper theme"
```

---

### 任务6：实现登录界面报纸风格

**文件：**
- 修改：`src/login_dialog.cpp`
- 修改：`include/login_dialog.h`
- 修改：`resources/style.qss`

- [ ] **步骤1：添加登录界面自定义绘制**

修改 `include/login_dialog.h`：

```cpp
#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPaintEvent>

class LoginDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog() override;
    
    QString username() const;
    QString serverAddress() const;
    quint16 serverPort() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onConnectClicked();

private:
    void setupUI();
    void drawNewspaperBackground(QPainter *painter);
    void drawGlobe(QPainter *painter);
    
    QLineEdit *m_usernameEdit;
    QLineEdit *m_serverEdit;
    QLineEdit *m_portEdit;
    QPushButton *m_connectButton;
};
```

- [ ] **步骤2：实现登录界面绘制**

修改 `src/login_dialog.cpp`：

```cpp
#include "login_dialog.h"
#include "font_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QSvgRenderer>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setFixedSize(440, 560);
}

LoginDialog::~LoginDialog() = default;

void LoginDialog::setupUI() {
    setWindowTitle("瓶子交流器 - 登录");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 80, 50, 50);
    
    // 标题
    QLabel *titleLabel = new QLabel("瓶子交流器", this);
    titleLabel->setFont(FontManager::instance().titleFont(22));
    titleLabel->setStyleSheet("color: #1c1917; font-weight: 600;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    mainLayout->addSpacing(30);
    
    // 用户名
    QLabel *usernameLabel = new QLabel("用户名", this);
    usernameLabel->setFont(FontManager::instance().bodyFont(13));
    usernameLabel->setStyleSheet("color: #78776c;");
    mainLayout->addWidget(usernameLabel);
    
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("输入用户名");
    m_usernameEdit->setFont(FontManager::instance().bodyFont());
    mainLayout->addWidget(m_usernameEdit);
    
    mainLayout->addSpacing(15);
    
    // 服务器地址
    QLabel *serverLabel = new QLabel("服务器地址", this);
    serverLabel->setFont(FontManager::instance().bodyFont(13));
    serverLabel->setStyleSheet("color: #78776c;");
    mainLayout->addWidget(serverLabel);
    
    m_serverEdit = new QLineEdit(this);
    m_serverEdit->setPlaceholderText("127.0.0.1");
    m_serverEdit->setFont(FontManager::instance().bodyFont());
    mainLayout->addWidget(m_serverEdit);
    
    mainLayout->addSpacing(15);
    
    // 端口
    QLabel *portLabel = new QLabel("端口", this);
    portLabel->setFont(FontManager::instance().bodyFont(13));
    portLabel->setStyleSheet("color: #78776c;");
    mainLayout->addWidget(portLabel);
    
    m_portEdit = new QLineEdit(this);
    m_portEdit->setPlaceholderText("8080");
    m_portEdit->setFont(FontManager::instance().bodyFont());
    mainLayout->addWidget(m_portEdit);
    
    mainLayout->addSpacing(30);
    
    // 连接按钮
    m_connectButton = new QPushButton("连接", this);
    m_connectButton->setFont(FontManager::instance().bodyFont(14));
    m_connectButton->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(m_connectButton);
    
    connect(m_connectButton, &QPushButton::clicked, this, &LoginDialog::onConnectClicked);
}

void LoginDialog::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制报纸风格背景
    drawNewspaperBackground(&painter);
    
    // 绘制地球仪
    drawGlobe(&painter);
    
    QDialog::paintEvent(event);
}

void LoginDialog::drawNewspaperBackground(QPainter *painter) {
    // 绘制报纸纹理背景
    painter->fillRect(rect(), QColor("#faf9f7"));
    
    // 细微的噪点效果
    for (int i = 0; i < 1000; i++) {
        int x = rand() % width();
        int y = rand() % height();
        painter->setPen(QPen(QColor("#d6d3d1"), 1));
        painter->drawPoint(x, y);
    }
}

void LoginDialog::drawGlobe(QPainter *painter) {
    // 加载并绘制地球仪SVG
    QSvgRenderer renderer(":/images/globe.svg");
    if (renderer.isValid()) {
        painter->setOpacity(0.07);
        QRectF targetRect(20, 20, 400, 400);
        renderer.render(painter, targetRect);
        painter->setOpacity(1.0);
    }
}

void LoginDialog::onConnectClicked() {
    if (m_usernameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入用户名");
        return;
    }
    accept();
}

QString LoginDialog::username() const {
    return m_usernameEdit->text();
}

QString LoginDialog::serverAddress() const {
    return m_serverEdit->text().isEmpty() ? "127.0.0.1" : m_serverEdit->text();
}

quint16 LoginDialog::serverPort() const {
    return m_portEdit->text().isEmpty() ? 8080 : m_portEdit->text().toUShort();
}
```

- [ ] **步骤3：更新样式表**

在 `resources/style.qss` 中添加登录界面样式：

```qss
/* 登录界面 */
LoginDialog QLineEdit {
    border: 1px solid #d6d3d1;
    border-radius: 2px;
    padding: 10px 12px;
    background-color: #ffffff;
    font-size: 15px;
}

LoginDialog QLineEdit:focus {
    border-color: #78776c;
}

LoginDialog QPushButton {
    background-color: #44403c;
    color: #ffffff;
    border: 1px solid #44403c;
    border-radius: 2px;
    padding: 10px;
    font-size: 14px;
    font-weight: 500;
}

LoginDialog QPushButton:hover {
    background-color: #57534e;
}
```

- [ ] **步骤4：编译验证**

运行：`cd D:\ChatBox\LinuxChat\client\build && cmake --build . --config Release`
预期：编译成功

- [ ] **步骤5：提交**

```bash
git add src/login_dialog.cpp include/login_dialog.h resources/style.qss
git commit -m "feat: implement login dialog with newspaper style"
```

---

### 任务7：实现主窗口布局

**文件：**
- 修改：`src/main_window.cpp`
- 修改：`include/main_window.h`
- 修改：`resources/style.qss`

- [ ] **步骤1：更新主窗口头文件**

修改 `include/main_window.h`：

```cpp
#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPaintEvent>

class ChatClient;
class ChatView;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(const QString &username, const QString &serverAddress, quint16 port, QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onSendMessage();
    void onDisconnect();
    void onUserListUpdated(const QStringList &users);

private:
    void setupUI();
    void drawWisteriaBackground(QPainter *painter);
    
    QString m_username;
    ChatClient *m_client;
    
    // UI组件
    QListWidget *m_userList;
    ChatView *m_chatView;
    QLineEdit *m_messageEdit;
    QPushButton *m_sendButton;
    QPushButton *m_disconnectButton;
    QLabel *m_statusLabel;
};
```

- [ ] **步骤2：实现主窗口布局**

修改 `src/main_window.cpp`：

```cpp
#include "main_window.h"
#include "chat_client.h"
#include "chat_view.h"
#include "font_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPainter>
#include <QSvgRenderer>

MainWindow::MainWindow(const QString &username, const QString &serverAddress, quint16 port, QWidget *parent)
    : QMainWindow(parent)
    , m_username(username)
    , m_client(new ChatClient(this))
{
    setupUI();
    setFixedSize(960, 640);
    setWindowTitle("瓶子交流器");
    
    // 连接到服务器
    m_client->connectToServer(serverAddress, port, username);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 顶部栏
    QHBoxLayout *topBar = new QHBoxLayout();
    topBar->setContentsMargins(16, 12, 16, 12);
    
    QLabel *logoLabel = new QLabel("瓶子交流器", this);
    logoLabel->setFont(FontManager::instance().titleFont(16));
    logoLabel->setStyleSheet("color: #1c1917; font-weight: 600;");
    topBar->addWidget(logoLabel);
    
    topBar->addStretch();
    
    m_statusLabel = new QLabel("已连接", this);
    m_statusLabel->setFont(FontManager::instance().bodyFont(11));
    m_statusLabel->setStyleSheet("color: #10b981;");
    topBar->addWidget(m_statusLabel);
    
    mainLayout->addLayout(topBar);
    
    // 分隔线
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #e7e5e4;");
    mainLayout->addWidget(separator);
    
    // 主体区域
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
    
    // 侧边栏
    QWidget *sidebar = new QWidget(this);
    sidebar->setFixedWidth(240);
    sidebar->setStyleSheet("background-color: #ffffff; border-right: 1px solid #e7e5e4;");
    
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(8, 8, 8, 8);
    
    QLabel *usersTitle = new QLabel("在线用户", this);
    usersTitle->setFont(FontManager::instance().bodyFont(13));
    usersTitle->setStyleSheet("color: #78776c; padding: 8px;");
    sidebarLayout->addWidget(usersTitle);
    
    m_userList = new QListWidget(this);
    m_userList->setFont(FontManager::instance().bodyFont());
    sidebarLayout->addWidget(m_userList);
    
    // 断开连接按钮
    m_disconnectButton = new QPushButton("断开连接", this);
    m_disconnectButton->setFont(FontManager::instance().bodyFont(11));
    m_disconnectButton->setStyleSheet(
        "QPushButton { background-color: transparent; border: 1px solid #e7e5e4; "
        "border-radius: 2px; padding: 6px 8px; color: #78776c; }"
        "QPushButton:hover { background-color: #f5f5f4; }"
    );
    sidebarLayout->addWidget(m_disconnectButton);
    
    bodyLayout->addWidget(sidebar);
    
    // 聊天区域
    QWidget *chatArea = new QWidget(this);
    QVBoxLayout *chatLayout = new QVBoxLayout(chatArea);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);
    
    m_chatView = new ChatView(this);
    chatLayout->addWidget(m_chatView);
    
    // 输入区域
    QWidget *inputArea = new QWidget(this);
    inputArea->setStyleSheet("background-color: #ffffff; border-top: 1px solid #e7e5e4;");
    QHBoxLayout *inputLayout = new QHBoxLayout(inputArea);
    inputLayout->setContentsMargins(16, 12, 16, 12);
    
    m_messageEdit = new QLineEdit(this);
    m_messageEdit->setPlaceholderText("输入消息...");
    m_messageEdit->setFont(FontManager::instance().bodyFont());
    inputLayout->addWidget(m_messageEdit);
    
    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setFont(FontManager::instance().bodyFont(13));
    m_sendButton->setStyleSheet(
        "QPushButton { background-color: #44403c; color: #ffffff; "
        "border: 1px solid #44403c; border-radius: 2px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #57534e; }"
    );
    inputLayout->addWidget(m_sendButton);
    
    chatLayout->addWidget(inputArea);
    
    bodyLayout->addWidget(chatArea);
    
    mainLayout->addLayout(bodyLayout);
    
    // 连接信号槽
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);
    connect(m_messageEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendMessage);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnect);
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制紫藤萝背景
    drawWisteriaBackground(&painter);
    
    QMainWindow::paintEvent(event);
}

void MainWindow::drawWisteriaBackground(QPainter *painter) {
    // 加载并绘制紫藤萝图案
    QSvgRenderer renderer(":/images/wisteria-pattern.svg");
    if (renderer.isValid()) {
        painter->setOpacity(0.35);
        
        // 平铺绘制
        QSize patternSize(300, 300);
        for (int x = 0; x < width(); x += patternSize.width()) {
            for (int y = 0; y < height(); y += patternSize.height()) {
                QRectF targetRect(x, y, patternSize.width(), patternSize.height());
                renderer.render(painter, targetRect);
            }
        }
        
        painter->setOpacity(1.0);
    }
}

void MainWindow::onSendMessage() {
    QString message = m_messageEdit->text().trimmed();
    if (!message.isEmpty()) {
        m_client->sendMessage(message);
        m_chatView->addMessage(m_username, message, true);
        m_messageEdit->clear();
    }
}

void MainWindow::onDisconnect() {
    m_client->disconnectFromServer();
    close();
}

void MainWindow::onUserListUpdated(const QStringList &users) {
    m_userList->clear();
    for (const QString &user : users) {
        m_userList->addItem(user);
    }
}
```

- [ ] **步骤3：更新样式表**

在 `resources/style.qss` 中添加主窗口样式：

```qss
/* 侧边栏用户列表 */
QListWidget {
    background-color: transparent;
    border: none;
    outline: none;
}

QListWidget::item {
    padding: 10px 12px;
    border-bottom: 1px solid #e7e5e4;
    color: #1c1917;
}

QListWidget::item:hover {
    background-color: #f5f5f4;
}

QListWidget::item:selected {
    background-color: #f0efed;
    border-left: 2px solid #44403c;
    padding-left: 10px;
    font-weight: 600;
    color: #1c1917;
}

/* 聊天区域 */
QLineEdit#messageEdit {
    border: 1px solid #d6d3d1;
    border-radius: 2px;
    padding: 10px 14px;
    background-color: #ffffff;
    font-size: 15px;
}

QLineEdit#messageEdit:focus {
    border-color: #78776c;
}
```

- [ ] **步骤4：编译验证**

运行：`cd D:\ChatBox\LinuxChat\client\build && cmake --build . --config Release`
预期：编译成功

- [ ] **步骤5：提交**

```bash
git add src/main_window.cpp include/main_window.h resources/style.qss
git commit -m "feat: implement main window with wisteria background"
```

---

### 任务8：实现聊天视图组件

**文件：**
- 修改：`src/chat_view.cpp`
- 修改：`include/chat_view.h`
- 修改：`resources/style.qss`

- [ ] **步骤1：更新聊天视图头文件**

修改 `include/chat_view.h`：

```cpp
#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

class ChatView : public QWidget {
    Q_OBJECT
    
public:
    explicit ChatView(QWidget *parent = nullptr);
    ~ChatView() override;
    
    void addMessage(const QString &sender, const QString &message, bool isSent);

private:
    void setupUI();
    
    QScrollArea *m_scrollArea;
    QWidget *m_messagesContainer;
    QVBoxLayout *m_messagesLayout;
};
```

- [ ] **步骤2：实现聊天视图**

修改 `src/chat_view.cpp`：

```cpp
#include "chat_view.h"
#include "font_manager.h"
#include <QLabel>
#include <QFrame>
#include <QScrollBar>

ChatView::ChatView(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

ChatView::~ChatView() = default;

void ChatView::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("background-color: transparent; border: none;");
    
    m_messagesContainer = new QWidget();
    m_messagesContainer->setStyleSheet("background-color: transparent;");
    m_messagesLayout = new QVBoxLayout(m_messagesContainer);
    m_messagesLayout->setContentsMargins(20, 20, 20, 20);
    m_messagesLayout->setSpacing(12);
    m_messagesLayout->addStretch();
    
    m_scrollArea->setWidget(m_messagesContainer);
    mainLayout->addWidget(m_scrollArea);
}

void ChatView::addMessage(const QString &sender, const QString &message, bool isSent) {
    // 创建消息容器
    QWidget *messageWidget = new QWidget();
    QHBoxLayout *messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    
    // 消息气泡
    QLabel *bubble = new QLabel(message);
    bubble->setFont(FontManager::instance().bodyFont(15));
    bubble->setWordWrap(true);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    if (isSent) {
        // 发送的消息 - 右对齐，深色背景
        bubble->setStyleSheet(
            "background-color: #44403c; color: #ffffff; "
            "border-radius: 2px; padding: 10px 14px;"
        );
        messageLayout->addStretch();
        messageLayout->addWidget(bubble);
    } else {
        // 接收的消息 - 左对齐，白色背景
        bubble->setStyleSheet(
            "background-color: #ffffff; color: #1c1917; "
            "border: 1px solid #e7e5e4; border-radius: 2px; padding: 10px 14px;"
        );
        messageLayout->addWidget(bubble);
        messageLayout->addStretch();
    }
    
    // 添加到布局
    m_messagesLayout->insertWidget(m_messagesLayout->count() - 1, messageWidget);
    
    // 滚动到底部
    QScrollBar *scrollBar = m_scrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}
```

- [ ] **步骤3：编译验证**

运行：`cd D:\ChatBox\LinuxChat\client\build && cmake --build . --config Release`
预期：编译成功

- [ ] **步骤4：提交**

```bash
git add src/chat_view.cpp include/chat_view.h
git commit -m "feat: implement chat view with message bubbles"
```

---

### 任务9：更新登录对话框集成到主窗口

**文件：**
- 修改：`main.cpp`

- [ ] **步骤1：更新main.cpp集成主窗口**

修改 `main.cpp`：

```cpp
#include <QApplication>
#include <QFile>
#include <QDebug>
#include "font_manager.h"
#include "login_dialog.h"
#include "main_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 加载自定义字体
    if (!FontManager::instance().loadFonts()) {
        qWarning() << "Failed to load custom fonts, using system defaults";
    }
    
    // 加载样式表
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = QString::fromUtf8(styleFile.readAll());
        app.setStyleSheet(style);
        qDebug() << "Style sheet loaded successfully";
    } else {
        qWarning() << "Failed to load style.qss:" << styleFile.errorString();
    }
    
    // 显示登录对话框
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        // 登录成功，创建并显示主窗口
        MainWindow mainWindow(
            loginDialog.username(),
            loginDialog.serverAddress(),
            loginDialog.serverPort()
        );
        mainWindow.show();
        
        return app.exec();
    }
    
    return 0;
}
```

- [ ] **步骤2：编译验证**

运行：`cd D:\ChatBox\LinuxChat\client\build && cmake --build . --config Release`
预期：编译成功

- [ ] **步骤3：提交**

```bash
git add main.cpp
git commit -m "feat: integrate login dialog with main window"
```

---

### 任务10：最终测试和优化

**文件：**
- 修改：`resources/style.qss`

- [ ] **步骤1：运行应用程序测试**

```bash
cd D:\ChatBox\LinuxChat\client\build
./Release/linuxchat_client.exe
```

预期：
- 登录界面显示报纸风格背景
- 地球仪SVG可见
- 字体正确加载
- 输入用户名后可以连接

- [ ] **步骤2：修复发现的问题**

根据测试结果修复样式或代码问题

- [ ] **步骤3：最终样式优化**

优化细节：
- 调整字体大小
- 优化颜色对比度
- 完善hover效果

- [ ] **步骤4：提交最终版本**

```bash
git add .
git commit -m "feat: complete UI redesign with newspaper and wisteria style"
```

---

## 执行交接

计划完成并保存到 `docs/superpowers/plans/2026-06-16-bottle-messenger-ui-redesign.md`。

**两种执行选项：**

**1. 子代理驱动（推荐）** - 我为每个任务调度一个新的子代理，任务之间审查，快速迭代

**2. 内联执行** - 使用executing-plans在此会话中执行任务，批量执行带检查点

**选择哪种方法？**
