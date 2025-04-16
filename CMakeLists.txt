cmake_minimum_required(VERSION 3.5)
project(qt_chat_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

cmake_policy(SET CMP0167 NEW)

find_package(Qt6 COMPONENTS Core Widgets Network REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_library(common_lib STATIC
    common/src/message.cpp
    common/include/message.h
)
target_include_directories(common_lib PUBLIC common/include)
target_link_libraries(common_lib PUBLIC Qt6::Core)

add_executable(chat_server
    server/src/main.cpp
    server/src/async_tcp_server.cpp
    server/include/async_tcp_server.h
    server/src/client_session.cpp
    server/include/client_session.h
)
target_include_directories(chat_server PRIVATE server/include common/include)
target_link_libraries(chat_server PRIVATE common_lib Qt6::Core Qt6::Network)

add_executable(chat_server_mt
    server/src/main_multithreaded.cpp
    server/src/multithreaded_server.cpp
    server/include/multithreaded_server.h
)
target_include_directories(chat_server_mt PRIVATE server/include common/include)
target_link_libraries(chat_server_mt PRIVATE common_lib Qt6::Core Boost::system)

add_executable(chat_client
    client/src/main.cpp
    client/src/chat_window.cpp
    client/include/chat_window.h
    client/src/network_client.cpp
    client/include/network_client.h
    client/src/network_client_factory.cpp
    client/include/network_client_factory.h
    client/include/inetwork_client.h
)
target_include_directories(chat_client PRIVATE client/include common/include)
target_link_libraries(chat_client PRIVATE common_lib Qt6::Core Qt6::Widgets Qt6::Network)