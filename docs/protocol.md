# LinuxChat — 通信协议规范

> 版本：1.0  
> 传输层：TCP 长连接（Linux 服务端 ↔ Windows Qt 客户端）

---

## 帧格式（解决 TCP 粘包）

每条消息 = **4 字节大端长度前缀** + **JSON 正文（UTF-8）**

```
┌─────────────────────┬────────────────────────────────────────┐
│  Length (4 bytes)   │  JSON Body (variable, UTF-8)           │
│  Big-Endian uint32  │  {"type":"...", "from":"...", ...}     │
└─────────────────────┴────────────────────────────────────────┘
```

### 发送伪代码
```cpp
std::string body = json_obj.dump();
uint32_t len = htonl(static_cast<uint32_t>(body.size()));
send(fd, &len, 4, 0);
send(fd, body.c_str(), body.size(), 0);
```

### 接收伪代码
```cpp
// 将数据追加到 ClientSession::recv_buf
// 循环检查：若 buf.size() >= 4，读出 len = ntohl(前4字节)
// 若 buf.size() >= 4 + len，提取完整 JSON 消息，移出缓冲区
```

---

## 基础消息结构

所有消息均为 JSON 对象，必须包含 `type` 字段：

```json
{
  "type":      "MSG_TYPE",     // 必填，见类型表
  "from":      "username",     // 发送方用户名（服务端填充或客户端提供）
  "to":        "username",     // 接收方（私聊时填，广播时省略）
  "content":   "消息文字",      // 消息内容
  "timestamp": 1700000000,     // Unix 时间戳（秒），服务端填充
  "code":      "ERROR_CODE",   // 仅 ERROR 消息使用
  "data":      {}              // 可选，附加载荷（如用户列表、历史记录）
}
```

---

## 消息类型表

### 客户端 → 服务端（C→S）

| type | 必填字段 | 说明 |
|------|---------|------|
| `REGISTER` | `from`(用户名), `content`(密码明文) | 注册新账号 |
| `LOGIN` | `from`(用户名), `content`(密码明文) | 登录 |
| `LOGOUT` | `from` | 主动退出 |
| `BROADCAST` | `from`, `content` | 发公共聊天室消息 |
| `PRIVATE` | `from`, `to`, `content` | 发私信 |
| `FRIEND_REQ` | `from`, `to` | 发送好友申请 |
| `FRIEND_ACK` | `from`, `to`, `content`("accept"\|"reject") | 回应好友申请 |
| `BLACKLIST_ADD` | `from`, `to` | 将 `to` 加入黑名单 |
| `BLACKLIST_RM` | `from`, `to` | 将 `to` 移出黑名单 |
| `HISTORY_REQ` | `from`, `to`("__room__" 或用户名) | 请求历史消息 |
| `PONG` | （无） | 心跳响应，收到 `PING` 后自动回复 |

### 服务端 → 客户端（S→C）

| type | 必填字段 | 说明 |
|------|---------|------|
| `LOGIN_OK` | `from`(自己用户名) | 登录/注册成功 |
| `BROADCAST` | `from`, `content`, `timestamp` | 广播消息（含历史推送）|
| `PRIVATE` | `from`, `to`, `content`, `timestamp` | 私信 |
| `USER_LIST` | `data`(在线用户名数组) | 在线用户列表推送 |
| `NOTIFY` | `content` | 系统通知（如"Alice 加入了聊天室"）|
| `FRIEND_REQ` | `from`, `to` | 好友申请通知 |
| `FRIEND_ACK` | `from`, `to`, `content` | 好友申请回应通知 |
| `HISTORY_RESP` | `to`, `data`(消息数组) | 历史消息列表 |
| `ERROR` | `code`, `content` | 错误通知 |
| `PING` | （无） | 心跳包，客户端收到后应回复 `PONG` |

---

## 错误码表（`ERROR` 消息的 `code` 字段）

| code | 含义 |
|------|------|
| `USER_EXISTS` | 注册时用户名已存在 |
| `USER_NOT_FOUND` | 用户不存在 |
| `WRONG_PASSWORD` | 密码错误 |
| `ALREADY_LOGGED_IN` | 该账号已在其他地方登录 |
| `TARGET_OFFLINE` | 私信目标用户不在线 |
| `BLOCKED` | 对方已将你拉入黑名单 |
| `NOT_AUTHENTICATED` | 未登录状态发送了需要认证的消息 |
| `INVALID_MSG` | 消息格式错误（缺少必填字段）|

---

## 典型交互流程

### 1. 注册 + 登录

```
Client                          Server
  │                               │
  ├──REGISTER{from:"alice",───────►│
  │   content:"pass123"}          │ 验证用户名唯一性
  │                               │ SHA-256 哈希密码
  │◄──────LOGIN_OK{from:"alice"}──┤ 存入 users 表
  │                               │ 注册在线状态
  │◄──────USER_LIST{data:[...]}───┤ 广播在线用户列表给所有人
  │◄──────BROADCAST(历史20条) ────┤ 推送最近公聊记录
```

### 2. 公共聊天室广播

```
Alice                  Server                   Bob
  │                      │                       │
  ├──BROADCAST{from:"alice",content:"hello"}────►│
  │                      │ 存入 messages 表       │
  │◄─────────────────────┤ 广播给所有在线用户      │
  │  BROADCAST{from:"alice",content:"hello",     │
  │            timestamp:1700000000}  ──────────►│
```

### 3. 私聊

```
Alice                  Server                   Bob
  │                      │                       │
  ├──PRIVATE{from:"alice",to:"bob",content:"hi"}►│
  │                      │ 检查 bob 是否在线      │
  │                      │ 检查黑名单             │
  │                      │ 存入 messages 表       │
  │                      │──PRIVATE{from:"alice",►│
  │                      │  to:"bob",content:"hi"}│
```

### 4. 好友申请流程

```
Alice                  Server                   Bob
  │                      │                       │
  ├──FRIEND_REQ{from:"alice",to:"bob"}──────────►│
  │                      │ 存入 friendships       │
  │                      │ 转发申请给 Bob          │
  │                      │──FRIEND_REQ{from:"alice",to:"bob"}──►│
  │                      │                       │
  │                      │◄──FRIEND_ACK{from:"bob",to:"alice",──┤
  │                      │     content:"accept"} │
  │                      │ 更新 friendships.status='accepted'   │
  │◄──FRIEND_ACK{from:"bob",to:"alice",...}──────┤
```

---

## USER_LIST 数据格式

```json
{
  "type": "USER_LIST",
  "data": ["alice", "bob", "charlie"],
  "timestamp": 1700000000
}
```

## HISTORY_RESP 数据格式

```json
{
  "type": "HISTORY_RESP",
  "to": "__room__",
  "data": [
    {"from": "alice", "content": "hello", "timestamp": 1699999990},
    {"from": "bob",   "content": "hi",    "timestamp": 1699999995}
  ]
}
```

## 心跳机制（PING/PONG）

为防止 NAT 超时或防火墙导致连接假死，服务端每 **30 秒**向所有在线客户端发送心跳包：

```
Server                              Client
  │                                   │
  ├──PING{}──────────────────────────►│
  │                                   │
  │◄──────────────────────────────────┤ PONG{}
```

- 服务端使用 `timerfd` 定时器在 epoll 事件循环中触发，广播 `{"type":"PING"}` 给所有已认证客户端
- 客户端收到 `PING` 后自动回复 `{"type":"PONG"}`
- 服务端收到 `PONG` 后静默处理（不转发、不记录）
- `PING`/`PONG` 消息体仅含 `type` 字段，无其他负载
