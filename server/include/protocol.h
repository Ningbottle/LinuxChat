#pragma once
// protocol.h — JSON-over-TCP framing: 4-byte big-endian length prefix + JSON body
//
// Frame format:
//   [uint32 big-endian length][JSON UTF-8 bytes...]
//
// Solves TCP stream fragmentation (粘包/半包) by buffering in ClientSession::recv_buf.

#include "client_session.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <optional>
#include <string>

namespace Protocol {

/// Serialize `msg` to JSON, prepend 4-byte big-endian length, send to `fd`.
/// Returns true on success, false if send failed (fd likely closed).
bool send_msg(int fd, const nlohmann::json& msg);

/// Convenience: send a simple response with type + optional fields.
bool send_error(int fd, const std::string& code, const std::string& content = "");
bool send_ok(int fd, const std::string& type, const std::string& from = "");

/// Read available bytes from `fd` into `session.recv_buf`.
/// Then extract all complete frames (length-prefixed JSON blobs).
/// Returns the list of fully-received JSON messages.
/// Returns empty vector if no complete frames yet.
/// Returns nullopt if the connection is closed/errored.
std::optional<std::vector<nlohmann::json>> recv_msgs(ClientSession& session);

} // namespace Protocol
