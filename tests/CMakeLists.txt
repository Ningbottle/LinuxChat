cmake_minimum_required(VERSION 3.16)
project(LinuxChatTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Google Test via FetchContent ──────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ── Paths ─────────────────────────────────────────────────────────
set(SERVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../server)
set(SERVER_INCLUDE ${SERVER_DIR}/include)
set(SERVER_THIRD_PARTY ${SERVER_DIR}/third_party)

# ── Common include dirs ──────────────────────────────────────────
include_directories(
    ${SERVER_INCLUDE}
    ${SERVER_THIRD_PARTY}
)

# ── Find dependencies ────────────────────────────────────────────
find_package(OpenSSL REQUIRED)

# ── Server source files (excluding main.cpp) ─────────────────────
set(SERVER_SOURCES
    ${SERVER_DIR}/src/protocol.cpp
    ${SERVER_DIR}/src/thread_pool.cpp
    ${SERVER_DIR}/src/database.cpp
    ${SERVER_DIR}/src/epoll_server.cpp
    ${SERVER_DIR}/src/message_router.cpp
)

# ── Test: Protocol ────────────────────────────────────────────────
add_executable(test_protocol
    test_protocol.cpp
    ${SERVER_DIR}/src/protocol.cpp
)
target_link_libraries(test_protocol
    GTest::gtest_main
    pthread
)

# ── Test: Database ────────────────────────────────────────────────
add_executable(test_database
    test_database.cpp
    ${SERVER_DIR}/src/database.cpp
)
target_link_libraries(test_database
    GTest::gtest_main
    sqlite3
    pthread
)

# ── Test: Message Handler ────────────────────────────────────────
add_executable(test_handler
    test_message_handler.cpp
    ${SERVER_SOURCES}
)
target_link_libraries(test_handler
    GTest::gtest_main
    sqlite3
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)

# ── Test: Crypto (SHA-256) ──────────────────────────────────────
add_executable(test_crypto
    test_crypto.cpp
    ${SERVER_DIR}/src/message_router.cpp
    ${SERVER_DIR}/src/database.cpp
    ${SERVER_DIR}/src/protocol.cpp
    ${SERVER_DIR}/src/epoll_server.cpp
    ${SERVER_DIR}/src/thread_pool.cpp
)
target_link_libraries(test_crypto
    GTest::gtest_main
    sqlite3
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)

# ── Test: Message Router ────────────────────────────────────────
add_executable(test_router
    test_message_router.cpp
    ${SERVER_SOURCES}
)
target_link_libraries(test_router
    GTest::gtest_main
    sqlite3
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)

# ── Test: Thread Pool ───────────────────────────────────────────
add_executable(test_thread_pool
    test_thread_pool.cpp
    ${SERVER_DIR}/src/thread_pool.cpp
)
target_link_libraries(test_thread_pool
    GTest::gtest_main
    pthread
)

# ── CTest integration ────────────────────────────────────────────
enable_testing()
include(GoogleTest)
gtest_discover_tests(test_protocol)
gtest_discover_tests(test_database)
gtest_discover_tests(test_handler)
gtest_discover_tests(test_crypto)
gtest_discover_tests(test_router)
gtest_discover_tests(test_thread_pool)
