cmake_minimum_required(VERSION 3.16)
project(LinuxChatServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -pthread")

# Include paths
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(linuxchat_server
    main.cpp
    ${SOURCES}
)

# Link libraries
find_package(OpenSSL REQUIRED)
find_package(spdlog REQUIRED)

target_link_libraries(linuxchat_server
    pthread
    sqlite3
    OpenSSL::SSL
    OpenSSL::Crypto
    spdlog::spdlog
)

# Optional: install
install(TARGETS linuxchat_server DESTINATION bin)
