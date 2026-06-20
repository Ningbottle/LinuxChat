cmake_minimum_required(VERSION 3.16)
project(LinuxChatClient CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Qt6 (QML-only, no Widgets)
find_package(Qt6 REQUIRED COMPONENTS Network Svg Qml Quick QuickControls2)

# Qt6 QML policy (QTP0004: qmldir for extra directories)
qt_policy(SET QTP0004 NEW)

# Qt settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Header files (MUST be listed for AUTOMOC to find Q_OBJECT)
file(GLOB_RECURSE HEADERS "include/*.h")

# Qt resources
set(RESOURCES resources/resources.qrc)

add_executable(linuxchat_client
    main.cpp
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# QML files loaded from file system at runtime — not compiled into binary
# This allows editing QML without rebuilding

# Include paths
target_include_directories(linuxchat_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/third_party   # shared nlohmann/json
)

# Link Qt6 (no Widgets) — must use PRIVATE to avoid keyword conflict with qt_add_qml_module
target_link_libraries(linuxchat_client PRIVATE
    Qt6::Network
    Qt6::Svg
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

# High DPI support & deprecation guards
target_compile_definitions(linuxchat_client PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)
