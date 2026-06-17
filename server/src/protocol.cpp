// protocol.cpp — JSON-over-TCP framing implementation

#include "protocol.h"
#include <sys/socket.h>
#include <arpa/inet.h>   // htonl, ntohl
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>

namespace Protocol {

// ── Helpers ───────────────────────────────────────────────────────────────

/// Write exactly `len` bytes to `fd`. Returns false on failure.
static bool write_all(int fd, const void* buf, size_t len) {
    const uint8_t* p = static_cast<const uint8_t*>(buf);
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(fd, p + sent, len - sent, MSG_NOSIGNAL);
        if (n <= 0) return false;
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
        std::cerr << "[Protocol] JSON serialization error: " << e.what() << "\n";
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
    // Read available bytes into session's buffer
    uint8_t tmp[4096];
    ssize_t n = ::recv(session.fd, tmp, sizeof(tmp), 0);

    if (n == 0) {
        // Connection closed by peer
        return std::nullopt;
    }
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data right now (non-blocking), not an error
            // Still try to parse what we have
        } else {
            return std::nullopt; // Real error
        }
    } else {
        // Append received bytes to session buffer
        session.recv_buf.insert(session.recv_buf.end(), tmp, tmp + n);
    }

    // Extract all complete frames from the buffer
    std::vector<nlohmann::json> messages;
    auto& buf = session.recv_buf;

    while (buf.size() >= 4) {
        // Read 4-byte big-endian length
        uint32_t net_len;
        std::memcpy(&net_len, buf.data(), 4);
        uint32_t body_len = ntohl(net_len);

        // Guard against absurdly large messages (e.g. 16MB)
        if (body_len > 16 * 1024 * 1024) {
            std::cerr << "[Protocol] fd=" << session.fd
                      << " sent oversized frame (" << body_len << " bytes), dropping.\n";
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
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "[Protocol] fd=" << session.fd
                      << " JSON parse error: " << e.what() << "\n";
            // Skip malformed message, continue
        }
    }

    return messages;
}

} // namespace Protocol
