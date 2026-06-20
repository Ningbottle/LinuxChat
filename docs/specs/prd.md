# PRD — LinuxChat (瓶子交流器 / Bottle Messenger)

Status: Active | Version: 1.0 | Last Updated: 2026-06-19

## Executive Summary

LinuxChat 是一个基于 **C/S 架构** 的网络即时通讯工具:Linux epoll 服务端 + Windows Qt6 客户端,通过自定义 JSON-over-TCP 协议(4 字节大端长度前缀)实现注册、登录、公聊、私聊、历史消息、在线用户列表等功能。本作品是《Linux 操作系统与程序设计》课程设计,要求融合 socket 网络通信、数据库访问、线程/进程通信、图形界面等技术。

## Problem

课设要求独立完成一个"基于 Linux 平台的较大类型应用程序"。需要一个能体现 socket + epoll + 多线程 + SQLite + Qt GUI 综合能力的可演示即时通讯系统,作为课程设计交付物。

## Goals

- G1 — 实现 C/S 架构即时通讯:Linux 服务端 + Windows 客户端,跨平台联通
- G2 — 自定义 JSON-over-TCP 协议(4 字节大端长度前缀),解决 TCP 粘包/半包
- G3 — 支持注册、登录、公聊广播、私聊、历史消息、在线用户列表
- G4 — 课设验收:程序可运行 + 完整文档(源码、说明、设计、测试)

## Non-goals

- NG1 — 好友系统、黑名单(protocol.md 中定义但非本期范围)
- NG2 — 离线消息、文件传输、图片传输
- NG3 — 群组聊天(本期只有公共聊天室 + 一对一私聊)
- NG4 — TLS/SSL 加密通道(本期明文 TCP,OpenSSL 仅用于 SHA-256 密码哈希)

## Users / Personas

- 学生/课设评审 — 通过公共聊天室和私聊验证系统功能完整性
- 普通用户 — 在 Windows 客户端注册账号、登录、收发消息

## User journeys

1) **注册→登录**:新用户在登录界面填用户名/密码 → 点"连接"建立 TCP → 点"注册" → 服务端建账 → 自动登录 → 进入主聊天窗口,看到历史消息和在线用户列表。
2) **公共聊天**:用户在"公共聊天室"标签输入消息 → 广播给所有在线用户 → 自己和他人都能看到。
3) **私聊**:双击侧边栏在线用户 → 打开私聊标签 → 发送私信 → 仅对方和自己可见。
4) **历史加载**:登录成功后,服务端推送最近 20 条公共聊天历史。

## Acceptance criteria

- AC1 — 服务端在 Linux 上编译并运行,监听 8080 端口
- AC2 — 客户端在 Windows(Qt6)上编译并运行
- AC3 — 所有协议测试(test_protocol.cpp)通过
- AC4 — 所有数据库测试(test_database.cpp)通过
- AC5 — 所有消息处理器测试(test_message_handler.cpp)通过
- AC6 — 注册 → 登录 → 广播 → 私聊 端到端流程可用
- AC7 — 登录后自动加载历史消息和在线用户列表
- AC8 — **连接稳定性:反复连接/断开不应出现"有时连得上有时连不上"或"连接成功后立即断开"**(见 Known Issues)

## Functional Requirements

- FR1 — REGISTER:服务端 SHA-256 哈希密码,存入 SQLite users 表
- FR2 — LOGIN:校验密码,标记在线,推送 USER_LIST + HISTORY_RESP + NOTIFY
- FR3 — BROADCAST:存库(to="__room__"),广播给所有在线用户
- FR4 — PRIVATE:存库,转发给目标在线用户,回显给发送者
- FR5 — LOGOUT/断连:清理在线状态,广播更新后的 USER_LIST
- FR6 — HISTORY_REQ:按 to(__room__ 或用户名)返回最近消息

## Non-functional Requirements

- NFR1 — 服务端用 epoll(level-triggered)+ 线程池,主线程 IO,worker 处理业务
- NFR2 — SQLite WAL 模式 + 互斥锁,支持并发读
- NFR3 — 帧格式:[4 字节大端 uint32 长度][JSON UTF-8 正文],16MB 上限
- NFR4 — 客户端当前运行入口为 Qt6 Widgets + QSS,主题为水墨国风；QML/Qt Quick 迁移脚手架为进行中工作,不替代当前验收路径

## Risks

| Risk | Impact | Mitigation |
|:-----|:-------|:-----------|
| 连接竞态(epoll 水平触发 + 异步 worker 数据错位) | 间歇性连接失败/登录后断开 | 见 Known Issues #1,TODO Step 01 |
| fd 复用导致 worker 串话 | 消息发错用户/use-after-close | 见 Known Issues #2,TODO Step 02 |
| OpenSSL 3.x 废弃 SHA256() API | 服务端编译失败/警告 | 见 Known Issues #3,TODO Step 03 |
| 客户端默认连公网 IP | 误以为代码 bug | 已确认是部署配置,非阻塞 |

## Known Issues (审计发现 — 2026-06-17)

> 以下问题由 CDD Phase 1 只读审计发现,详见 `docs/JOURNAL.md` 首条记录。优先级排序见 TODO Step 01-03。

### #1 [Critical] epoll 水平触发 + 非阻塞 socket + worker 异步处理 → 数据错位/丢失
- 位置:`server/src/epoll_server.cpp:191,213-257` + `server/src/protocol.cpp:60-117`
- 现象:客户端"有时连得上有时连不上";登录瞬间(服务端回 LOGIN_OK+HISTORY+USER_LIST+NOTIFY 多帧)最易触发
- 根因:`recv_msgs` 单次 recv 4096B;水平触发下未读完的数据立即再次就绪;多帧错位拼接 → 长度前缀读错 → oversized 判定(protocol.cpp:92)→ `return nullopt` → 连接被踢

### #2 [Critical] session 生命周期竞态:worker 持 fd 但 fd 已被复用
- 位置:`server/src/epoll_server.cpp:235-290`
- 现象:反复重连时(同一 fd 数字频繁复用)偶发串话/数据写错地方
- 根因:worker 按值捕获 fd,运行时按 fd 查 session;旧 fd 被 close 后新连接复用同号 fd → worker 操作到错误 session

### #3 [High] OpenSSL 3.x 下 SHA256() 废弃 API
- 位置:`server/main.cpp:36-47`
- 影响:新发行版编译告警/潜在符号问题

### #4 [Medium] 客户端连接超时定时器语义混淆
- 位置:`client/src/login_dialog.cpp:46-57,282-288`
- 影响:`is_connected()` 判断 TCP 层而非应用层登录态;登录卡住时无独立超时

## Open Questions

1) 课设原文要求"Linux 下 gnome 图形界面",但客户端实现为 Windows Qt6。是否需要在文档中作为"已知偏差"说明,还是改回 Linux gnome?(架构 C/S 不变)
2) 是否需要支持多服务器/多房间扩展(非本期范围,但课设答辩可能被问到)?
