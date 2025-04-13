cmake_minimum_required(VERSION 3.16)
project(qt_chat_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Найти Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets)

# Общая библиотека
add_library(common_lib STATIC
    common/src/message.cpp
    common/include/message.h
)

target_include_directories(common_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(common_lib PUBLIC
    Qt6::Core
)

# Сервер
add_executable(chat_server
    server/src/main.cpp
    server/src/async_tcp_server.cpp
    server/src/client_session.cpp
    server/include/async_tcp_server.h
    server/include/client_session.h
)

target_include_directories(chat_server PRIVATE
    ${CMAKE_SOURCE_DIR}/server/include
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(chat_server PRIVATE
    Qt6::Core
    Qt6::Network
    common_lib
)

# Клиент
add_executable(chat_client
    client/src/main.cpp
    client/src/chat_window.cpp
    client/src/network_client.cpp
    client/include/chat_window.h
    client/include/network_client.h
)

target_include_directories(chat_client PRIVATE
    ${CMAKE_SOURCE_DIR}/client/include
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(chat_client PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    common_lib
)