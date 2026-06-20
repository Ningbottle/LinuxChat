// protocol.cpp — JSON-over-TCP framing implementation

#include "protocol.h"
#include "log_utils.h"
#include <sys/socket.h>
#include <arpa/inet.h>   // htonl, ntohl
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <thread>
#include <chrono>

namespace Protocol {

// ── Helpers ───────────────────────────────────────────────────────────────

/// Write exactly `len` bytes to `fd`. Returns false on failure.
/// Retries on EAGAIN/EWOULDBLOCK with a 5-second total timeout to prevent
/// thread pool starvation from a stuck/slow client.
///
/// On timeout or error, calls shutdown(fd, SHUT_WR) to prevent the caller
/// from sending more data on a corrupted TCP stream. If partial data was
/// already sent, the peer will see a protocol desync — the only correct
/// response is to tear down the connection.
static bool write_all(int fd, const void* buf, size_t len) {
    const uint8_t* p = static_cast<const uint8_t*>(buf);
    size_t sent = 0;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (sent < len) {
        ssize_t n = ::send(fd, p + sent, len - sent, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (std::chrono::steady_clock::now() >= deadline) {
                    std::cerr << "[Protocol] write_all timeout (5s) on fd=" << fd << "\n";
                    ::shutdown(fd, SHUT_WR);  // prevent further sends on corrupted stream
                    return false;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            return false;  // real error — socket already broken
        }
        if (n == 0) return false;  // unexpected zero-byte send
        sent += static_cast<size_t>(n);
    }
    return true;
}

// ── Public API ────────────────────────────────────────────────────────────

bool send_msg(int fd, const nlohmann::json& msg) {
    std::string body;
    try {
        body = msg.dump();
    } catch (const std::exception& e) {
        std::cerr << now_stamp() << " [Protocol] JSON serialization error: " << e.what() << "\n";
        return false;
    }

    // Guard against oversized outbound frames (defense-in-depth)
    constexpr size_t MAX_BODY_LEN = 256 * 1024;
    if (body.size() > MAX_BODY_LEN) {
        std::cerr << now_stamp() << " [Protocol] send_msg: body too large ("
                  << body.size() << " bytes), refusing to send.\n";
        return false;
    }

    // 4-byte big-endian length prefix
    uint32_t net_len = htonl(static_cast<uint32_t>(body.size()));
    if (!write_all(fd, &net_len, 4)) return false;
    if (!write_all(fd, body.c_str(), body.size())) return false;
    return true;
}

bool send_error(int fd, const std::string& code, const std::string& content) {
    nlohmann::json msg = {
        {"type",    "ERROR"},
        {"code",    code},
        {"content", content.empty() ? code : content}
    };
    return send_msg(fd, msg);
}

bool send_ok(int fd, const std::string& type, const std::string& from) {
    nlohmann::json msg = {{"type", type}};
    if (!from.empty()) msg["from"] = from;
    return send_msg(fd, msg);
}

std::optional<std::vector<nlohmann::json>> recv_msgs(ClientSession& session) {
    // Drain all available bytes for level-triggered epoll (loop until EAGAIN).
    // Prevents partial frames / length-prefix desync across multiple wakeups.
    bool peer_closed = false;
    uint8_t tmp[4096];

    // Maximum recv_buf size: 1MB. Prevents DoS from clients that send data
    // without proper framing, causing unbounded memory growth.
    constexpr size_t MAX_RECV_BUF_SIZE = 1024 * 1024;

    for (;;) {
        // Check recv_buf size before adding more data
        if (session.recv_buf.size() >= MAX_RECV_BUF_SIZE) {
            std::cerr << now_stamp() << " [Protocol] fd=" << session.fd
                      << " recv_buf too large (" << session.recv_buf.size()
                      << " bytes), dropping connection.\n";
            return std::nullopt;
        }

        ssize_t n = ::recv(session.fd, tmp, sizeof(tmp), 0);
        if (n > 0) {
            session.recv_buf.insert(session.recv_buf.end(), tmp, tmp + n);
            continue;  // try to drain more
        }
        if (n == 0) {
            // Connection closed by peer (FIN received).
            // Mark and break to process any buffered data first.
            std::cout << now_stamp() << " [Protocol] fd=" << session.fd << " recv n=0 (peer closed from client side)" << std::endl;
            peer_closed = true;
            break;
        }
        // n < 0
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;  // no more data right now; parse what we have
        }
        std::cout << now_stamp() << " [Protocol] fd=" << session.fd << " recv error: " << strerror(errno) << std::endl;
        return std::nullopt; // real error
    }

    // Extract all complete frames from the buffer
    std::vector<nlohmann::json> messages;
    auto& buf = session.recv_buf;

    while (buf.size() >= 4) {
        // Read 4-byte big-endian length
        uint32_t net_len;
        std::memcpy(&net_len, buf.data(), 4);
        uint32_t body_len = ntohl(net_len);

        // Guard against zero-length body
        if (body_len == 0) {
            std::cerr << now_stamp() << " [Protocol] fd=" << session.fd
                      << " sent zero-length body, dropping.\n";
            buf.clear();
            return std::nullopt;
        }

        // Guard against oversized messages (256KB limit for chat protocol).
        // Close the connection: oversized claim is either attack or unrecoverable desync.
        constexpr uint32_t MAX_BODY_LEN = 256 * 1024;
        if (body_len > MAX_BODY_LEN) {
            std::cerr << now_stamp() << " [Protocol] fd=" << session.fd
                      << " sent oversized frame (" << body_len << " bytes), dropping.\n";
            buf.clear();
            return std::nullopt;
        }

        // Check if we have the full body
        if (buf.size() < 4 + body_len) {
            break; // Incomplete frame — wait for more data
        }

        // Extract the JSON body
        std::string json_str(reinterpret_cast<char*>(buf.data() + 4), body_len);
        buf.erase(buf.begin(), buf.begin() + 4 + body_len);

        try {
            messages.push_back(nlohmann::json::parse(json_str));
            session.consecutive_parse_failures = 0;  // Reset on success
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << now_stamp() << " [Protocol] fd=" << session.fd
                      << " JSON parse error: " << e.what() << "\n";
            // Disconnect after 3 consecutive parse failures (malicious client)
            if (++session.consecutive_parse_failures >= 3) {
                std::cerr << now_stamp() << " [Protocol] fd=" << session.fd
                          << " too many parse failures, dropping connection.\n";
                return std::nullopt;
            }
        }
    }

    // If peer closed (FIN), return messages if any were parsed, else nullopt
    if (peer_closed) {
        return messages.empty() ? std::nullopt : std::make_optional(std::move(messages));
    }

    return messages;
}

} // namespace Protocol
