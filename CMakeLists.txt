cmake_minimum_required(VERSION 3.5)
project(qt_chat_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

cmake_policy(SET CMP0167 NEW)

find_package(Qt6 COMPONENTS Core Widgets Network Sql REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_library(common_lib STATIC
    common/src/message.cpp
    common/include/message.h
    common/src/database_manager.cpp
    common/include/database_manager.h
)
target_include_directories(common_lib PUBLIC common/include)
target_link_libraries(common_lib PUBLIC Qt6::Core Qt6::Sql)

add_executable(chat_server_mt
    server/src/main_multithreaded.cpp
    server/src/multithreaded_server.cpp
    server/include/multithreaded_server.h  
    server/src/boost_asio_client_session.cpp
    server/include/boost_asio_client_session.h  
    server/include/client_session_interface.h
    server/include/iserver.h
    server/src/client_manager.cpp
    server/include/client_manager.h
    server/src/message_handler.cpp
    server/include/message_handler.h
)
target_include_directories(chat_server_mt PRIVATE server/include common/include)
target_link_libraries(chat_server_mt PRIVATE common_lib Qt6::Core Qt6::Network Qt6::Sql)

add_executable(chat_client
    client/src/main.cpp
    client/src/chat_window.cpp
    client/include/chat_window.h
    client/src/network_client.cpp
    client/include/network_client.h
    client/src/network_client_factory.cpp
    client/include/network_client_factory.h
    client/include/inetwork_client.h
    client/src/chat_controller.cpp
    client/include/chat_controller.h
    client/src/auth_handler.cpp
    client/include/auth_handler.h
    client/src/emoji_window.cpp  
    client/include/emoji_window.h  
    client/src/login_window.cpp
    client/include/login_window.h
    client/src/password_window.cpp
    client/include/password_window.h
    client/src/register_window.cpp
    client/include/register_window.h
    client/src/user_list_window.cpp
    client/include/user_list_window.h
)
target_include_directories(chat_client PRIVATE client/include common/include)
target_link_libraries(chat_client PRIVATE common_lib Qt6::Core Qt6::Widgets Qt6::Network Qt6::Sql)