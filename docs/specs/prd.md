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
- NFR4 — 客户端运行入口为 Qt6 QML (Ethereal Frosted Glass 主题),具有水墨国风背景和毛玻璃质感，提供现代化动画体验。

## Risks

| Risk | Impact | Mitigation |
|:-----|:-------|:-----------|
| OpenSSL 3.x 废弃 SHA256() API | 服务端编译失败/警告 | 见 Known Issues #1,TODO Step 03 |
| 客户端默认连公网 IP | 误以为代码 bug | 已确认是部署配置,非阻塞 |

## Known Issues (剩余项)

### #1 [Resolved] OpenSSL 3.x 下 SHA256() 废弃 API
- 已通过迁移至 `EVP_MD` 接口解决。

## Open Questions

1) 课设原文要求"Linux 下 gnome 图形界面",但客户端实现为 Windows Qt6。是否需要在文档中作为"已知偏差"说明,还是改回 Linux gnome?(架构 C/S 不变)
2) 是否需要支持多服务器/多房间扩展(非本期范围,但课设答辩可能被问到)?
