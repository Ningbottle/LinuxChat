# LinuxChat 课程设计展示

> 《Linux 操作系统与程序设计》课程设计 — 5 分钟展示脚本

---

## 1. 项目概述（30秒）

**LinuxChat** — 一个 C/S 架构的即时通讯系统

- **服务端**: Linux epoll TCP 服务器，C++17
- **客户端**: Windows Qt6 桌面程序
- **协议**: 自定义 JSON-over-TCP（4 字节大端长度前缀 + JSON body）
- **风格**: Glassmorphism 暗色主题（毛玻璃效果 + Indigo 强调色）

**核心功能**: 注册、登录、公共聊天室广播、一对一私聊、历史消息查询、在线用户列表

---

## 2. 技术架构（1分钟）

### 服务端架构

```
main.cpp (入口)
  ├── Database        ← SQLite3 WAL 模式，mutex 保护
  ├── EpollServer     ← epoll LT 事件循环 + ThreadPool
  └── MessageRouter   ← 消息路由中心（430行 → 113行重构）
```

**关键技术栈**:
| 组件 | 技术 |
|------|------|
| 事件模型 | epoll level-triggered |
| 线程模型 | 主线程 epoll_wait + N 个 worker 线程 |
| 数据库 | SQLite3 WAL 模式（支持并发读） |
| 加密 | OpenSSL 3.x EVP API（SHA-256） |
| JSON | nlohmann/json（header-only） |

### 客户端架构

```
MainWindow (主窗口)
  ├── ChatClient ← QTcpSocket + JSON 编解码
  ├── LoginDialog ← 登录/注册界面
  ├── ChatView ← 消息气泡 + 输入区
  └── Sidebar ← 在线用户列表 + 会话切换
```

### 通信协议

```
┌──────────────┬────────────────────┐
│ 4B BE Length │ JSON Body          │
│ (大端序)     │ (最大 16MB)        │
└──────────────┴────────────────────┘

消息类型: REGISTER / LOGIN / BROADCAST / PRIVATE / HISTORY_REQ / USER_LIST / NOTIFY / ERROR
```

---

## 3. 核心功能演示（2分钟）

### 3.1 用户注册/登录

```
Client                          Server
  ├──REGISTER{from,pwd}──────────►│
  │                               │ SHA-256(password)
  │                               │ db.register_user()
  │◄──────LOGIN_OK{from}──────────│ finish_login()
  │◄──────USER_LIST{data:[...]}───│ broadcast_user_list()
  │◄──────HISTORY_RESP{data:[]}───│ push_history()
```

**演示要点**: 注册后自动登录，立即收到历史消息和在线用户列表

### 3.2 公共聊天室广播

```
Alice                   Server                    Bob
  ├──BROADCAST{content}──►│                        │
  │                       │ db.store_message()     │
  │◄──────────────────────│ broadcast(msg)         │
  │  BROADCAST{from,      │                        │
  │   content, ts}────────┼───────────────────────►│
```

**演示要点**: 消息持久化到数据库，所有在线用户实时收到

### 3.3 私聊消息

```
Alice                   Server                    Bob
  ├──PRIVATE{to,content}─►│                        │
  │                       │ db.store_message()     │
  │                       │ send_to_fd(bob_fd)     │
  │◄──────────────────────│ send_to_fd(alice_fd)   │
  │  PRIVATE{from,to,...}─┼───────────────────────►│
```

**演示要点**: 私聊消息只发送给目标用户，同时回显给发送者

### 3.4 历史消息查询

```
Client                          Server
  ├──HISTORY_REQ{to}─────────────►│
  │                               │ db.get_history(to, 50)
  │◄──────HISTORY_RESP{data:[...]}│
```

**演示要点**: 支持公共聊天室历史和私聊历史，登录时自动加载最近 20 条

### 3.5 在线用户列表

- 用户连接/断开时自动广播 USER_LIST
- 客户端实时更新侧边栏用户列表
- 支持检测重复登录（同一账号多地登录）

---

## 4. 技术亮点（1分钟）

### 4.1 MessageRouter 架构重构

**问题**: main.cpp 430 行，消息处理、数据库操作、在线管理混杂

**方案**: 提取 MessageRouter 类
- 430 行 → 113 行（main.cpp 精简为纯入口）
- 消息路由逻辑独立，可单元测试
- 27 个路由测试覆盖所有分支

### 4.2 shared_ptr 消除 use-after-free

**问题**: worker 线程持有裸 session 指针，主线程可能 close fd 并释放 session

**方案**: `sessions_` 从 `unordered_map<int, ClientSession*>` 改为 `unordered_map<int, shared_ptr<ClientSession>>`
- worker lambda 捕获 shared_ptr，延长生命周期
- 配合 generation token 防止 fd 复用串话

### 4.3 broadcast 锁优化

**问题**: broadcast 在持锁期间发送，阻塞主线程 accept

**方案**: 复制-释放-发送策略
```cpp
void broadcast(const json& msg) {
    std::vector<int> fds;
    {
        std::lock_guard lock(sessions_mutex_);
        // 复制 fd 列表
        for (auto& [fd, session] : sessions_) {
            if (session->is_authenticated()) fds.push_back(fd);
        }
    }
    // 释放锁后发送
    for (int fd : fds) send_to_fd(fd, msg);
}
```

### 4.4 write_all EAGAIN 重试

**问题**: 大消息发送时可能部分写入，直接返回导致数据丢失

**方案**: `send_all` 循环处理 EAGAIN/EINTR
```cpp
while (sent < total) {
    ssize_t n = ::send(fd, buf + sent, total - sent, MSG_NOSIGNAL);
    if (n < 0) {
        if (errno == EAGAIN || errno == EINTR) continue;  // 重试
        return false;  // 真错误
    }
    sent += n;
}
```

### 4.5 108 个单元测试

| 模块 | 测试数 | 覆盖内容 |
|------|--------|----------|
| test_protocol | 19 | 帧编解码、边界情况、错误处理 |
| test_database | 27 | CRUD、历史查询、并发安全 |
| test_message_handler | 15 | 处理器业务逻辑 |
| test_message_router | 27 | 路由分发、认证、在线管理 |
| test_crypto | 9 | SHA-256 EVP API |
| test_thread_pool | 11 | 线程池任务分发、并发 |

---

## 5. Q&A 准备（30秒）

### Q1: 为什么用 epoll 而不是 select/poll？

**A**: 
- select: FD_SETSIZE 限制（通常 1024），O(n) 遍历
- poll: 无 FD_SETSIZE 限制，但仍 O(n) 遍历
- epoll: O(1) 事件通知，支持百万级连接
- 课程项目规模用 select/poll 也够，但 epoll 是 Linux 高并发标准方案

### Q2: 如何处理并发连接？

**A**:
- 主线程：epoll_wait() 接受连接、读取数据（非阻塞 I/O）
- Worker 线程池：解析 JSON、执行业务逻辑、调用 Database
- sessions_mutex_ 保护连接表，粒度细（只在查找/删除时持锁）
- shared_ptr 保证 worker 安全访问 session

### Q3: 消息如何保证可靠传输？

**A**:
- TCP 保证字节流可靠传输（重传、排序、流控）
- 应用层：4 字节长度前缀解决 TCP 粘包/拆包
- send_all 循环处理 EAGAIN，确保完整写入
- 注：当前无应用层 ACK，未来可扩展

### Q4: 为什么选择 SQLite 而不是 MySQL？

**A**:
- 零配置：无需安装数据库服务器
- 单文件：`linuxchat.db` 便于备份和迁移
- WAL 模式：支持并发读，写入不阻塞读取
- 课程项目规模足够，性能无瓶颈
- 缺点：不适合多服务器部署（未来可迁移到 MySQL）

### Q5: 如何扩展为支持文件传输？

**A**:
1. 协议层：新增 `FILE_TRANSFER` 消息类型
2. 方案 A（小文件）：Base64 编码放入 JSON body
3. 方案 B（大文件）：独立 TCP 连接或 UDP 传输，JSON 只传元数据
4. 服务端：存储文件到磁盘或对象存储，返回文件 ID
5. 客户端：发送文件 ID，接收方通过 ID 下载

---

## 展示流程建议

| 时间 | 内容 | 重点 |
|------|------|------|
| 0:00-0:30 | 项目概述 | C/S 架构、技术栈 |
| 0:30-1:30 | 技术架构 | 服务端/客户端/协议图解 |
| 1:30-3:30 | 功能演示 | 注册→登录→广播→私聊→历史→用户列表 |
| 3:30-4:30 | 技术亮点 | MessageRouter、shared_ptr、broadcast 优化、测试 |
| 4:30-5:00 | Q&A | 准备 5 个常见问题 |

---

## 演示脚本

### 准备工作
1. 启动服务端: `./linuxchat_server --port 8080`
2. 启动客户端 A: `./linuxchat_client.exe` (登录 Alice)
3. 启动客户端 B: `./linuxchat_client.exe` (登录 Bob)

### 演示步骤
1. **注册**: Alice 注册新账号，展示自动登录 + 历史消息
2. **登录**: Bob 登录，展示 USER_LIST 更新
3. **广播**: Alice 发送公共消息，Bob 实时收到
4. **私聊**: Alice 私聊 Bob，展示私聊消息
5. **历史**: 切换会话，展示历史消息加载
6. **用户列表**: 展示在线用户实时更新
7. **断开重连**: 展示 generation token 防止串话

### QSS 展示（可选）
```bash
# 展示 Glassmorphism 暗色主题
./linuxchat_client.exe --test-chat
```
