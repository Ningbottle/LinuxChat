#pragma once
// client_session.h — Per-connection state for each TCP client

#include <string>
#include <vector>
#include <cstdint>

/// Holds all state associated with a single connected client.
/// One ClientSession is created per accepted fd and destroyed on disconnect.
struct ClientSession {
    int         fd;           ///< Socket file descriptor
    std::string username;     ///< Empty = not yet authenticated
    std::vector<uint8_t> recv_buf; ///< Accumulates raw bytes for frame extraction
    uint64_t    generation = 0; ///< Incremented per new connection on this fd number (detect reuse)

    explicit ClientSession(int fd) : fd(fd) {}

    bool is_authenticated() const { return !username.empty(); }
};
