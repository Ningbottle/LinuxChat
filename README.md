# 瓶子交流器 (Bottle Messenger)

一个基于 Qt 6 和 C++ 的即时通讯应用，采用报纸复古 + 紫藤萝瀑布风格设计。

## 项目结构

```
LinuxChat/
├── client/                 # Qt 6 客户端
│   ├── include/           # 头文件
│   ├── src/               # 源文件
│   ├── resources/         # 资源文件
│   │   ├── fonts/        # 自定义字体（LXGW WenKai、Newsreader）
│   │   ├── images/       # SVG 图片（地球仪、紫藤萝）
│   │   ├── style.qss    # Qt 样式表
│   │   └── resources.qrc # Qt 资源文件
│   └── CMakeLists.txt
├── server/                 # C++ 服务端
│   ├── include/           # 头文件
│   ├── src/               # 源文件
│   └── CMakeLists.txt
├── tests/                  # 测试文件
└── docs/                   # 文档
```

## 设计风格

### 登录界面
- **报纸复古风格**：米色背景 + 细微噪点纹理
- **地球仪 SVG**：低透明度线条风格地球仪背景
- **字体**：LXGW WenKai（霞鹜文楷）正文 + Newsreader 标题

### 主聊天界面
- **紫藤萝瀑布背景**：宗璞《紫藤萝瀑布》风格
  - 棕色藤蔓 + 淡紫色花穗 + 绿色叶子
  - SVG 平铺图案，opacity 0.35
- **配色**：石墨灰系（#44403c、#78776c、#faf9f7）

## 快速开始

### Windows 客户端

#### 前置要求
- Qt 6.8.0 或更高版本
- Visual Studio 2022 或 CMake 3.16+
- Qt MSVC 2022 64-bit 组件

#### 编译
```powershell
cd client
mkdir build
cd build
cmake .. -G "Visual Studio 18 2026" -DCMAKE_PREFIX_PATH="D:/Qt/6.8.0/msvc2022_64"
cmake --build . --config Release
```

#### 运行
```powershell
cd Release
./linuxchat_client.exe
```

### Linux 服务端

#### 前置要求
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake libsqlite3-dev libssl-dev

# CentOS/RHEL
sudo yum install gcc-c++ cmake sqlite-devel openssl-devel
```

#### 编译
```bash
cd server
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### 运行
```bash
# 默认端口 8080
./linuxchat_server

# 自定义端口
./linuxchat_server --port 9090

# 查看帮助
./linuxchat_server --help
```

## 连接服务器

在客户端登录界面：
- **服务器地址**：Linux 机器的 IP 地址
- **端口**：8080（或自定义端口）
- **用户名**：任意用户名

## 技术栈

### 客户端
- **Qt 6.8.0**：GUI 框架
- **QSS**：样式表
- **QPainter + SVG**：自定义绘制
- **C++17**

### 服务端
- **C++17**
- **Epoll**：高性能 IO 多路复用
- **SQLite**：数据存储
- **OpenSSL**：加密通信
- **线程池**：并发处理

## 设计文档

- [UI 设计规范](docs/superpowers/specs/2026-06-16-bottle-messenger-ui-design.md)
- [实现计划](docs/superpowers/plans/2026-06-16-bottle-messenger-ui-redesign.md)
- [通信协议](docs/protocol.md)

## 特性

- ✅ 报纸复古风格登录界面
- ✅ 紫藤萝瀑布聊天背景
- ✅ 自定义字体支持（LXGW WenKai、Newsreader）
- ✅ 消息气泡（发送/接收）
- ✅ 在线用户列表
- ✅ 当前会话高亮
- ✅ 公聊和私聊支持
- ✅ 跨平台（Windows 客户端 + Linux 服务端）

## 许可证

MIT License

## 作者

Ningbottle
